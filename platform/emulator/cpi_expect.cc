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

#include "cpi.hh"
#include "var_base.hh"
#include "var_ct.hh"
#include "var_fd.hh"
#include "var_bool.hh"
#include "var_fs.hh"
#include "var_of.hh"
#include "prop_int.hh"

void splitfname(const char * fname, char * &dirname, char * &basename)
{
  const char delim_char = '/';

  const int splitlen = 1024;
  static char split[splitlen];
  static char * empty = "";


  if (strlen(fname) > splitlen-1) {
    basename = dirname = empty;
  } else {
    strcpy(split, fname);

    char * delim = strrchr(split, delim_char);

    if (delim == NULL) { // '/' not found
    dirname = empty;
    basename = split;
    } else {
      dirname = split;
      basename = delim+1;
      *delim = '\0';
    }
  }
}


inline
OZ_expect_t expectFail(void) {
  return OZ_expect_t(0, -1);
}

inline
OZ_expect_t expectProceed(int s, int a) {
  return OZ_expect_t(s, a);
}

inline
OZ_expect_t expectSuspend(int s, int a)
{
  Assert(s > a);
  return OZ_expect_t(s, a);
}

inline
OZ_expect_t expectExceptional(void) {
  return OZ_expect_t(0, -2);
}

// expect_t.accepted:
//   -1 on failure, at least one term is inconsistent
//    0 on unsuffient constraints
// >= 1 number of terms sufficiently constrained
// args are sufficiently constrained if expect_t.size == expect_t.accepted

//-----------------------------------------------------------------------------

OZ_Expect::OZ_Expect(void)
{
  staticSpawnVarsNumber = staticSuspendVarsNumber = 0;
  collect = TRUE;
}

OZ_Expect::~OZ_Expect(void)
{
}

void OZ_Expect::collectVarsOn(void)
{
  collect = TRUE;
}

void OZ_Expect::collectVarsOff(void)
{
  collect = FALSE;
}

// variables are suffiently constrained
inline
void OZ_Expect::addSpawnBool(OZ_Term * v)
{
  if (collect)
    staticAddSpawnBool(v);
}

inline
void OZ_Expect::addSpawn(OZ_FDPropState ps, OZ_Term * v)
{
  if (collect)
    staticAddSpawn(ps, v);
}

inline
void OZ_Expect::addSpawn(OZ_FSetPropState ps, OZ_Term * v)
{
  if (collect)
    staticAddSpawn(ps, v);
}

inline
void OZ_Expect::addSpawn(OZ_CtDefinition * def, OZ_CtWakeUp w, OZ_Term * v)
{
  if (collect)
    staticAddSpawn(def, w, v);
}

// variables are insuffiently constrained

inline
void OZ_Expect::addSuspend(OZ_Term * v)
{
  if (collect) {
    staticSuspendVars[staticSuspendVarsNumber].var = v;
    DebugCode(staticSuspendVars[staticSuspendVarsNumber].expected_type = OZ_VAR_INVALID);
    staticSuspendVarsNumber++;

    staticSuspendVars.request(staticSuspendVarsNumber);
  }
}

// boolean finite domain constraints
inline
void OZ_Expect::addSuspendBool(OZ_Term * v)
{
  if (collect) {
    staticSuspendVars[staticSuspendVarsNumber].var = v;
    staticSuspendVars[staticSuspendVarsNumber].expected_type = OZ_VAR_BOOL;
    staticSuspendVarsNumber++;

    staticSuspendVars.request(staticSuspendVarsNumber);
  }
}

// finite domain constraints
inline
void OZ_Expect::addSuspend(OZ_FDPropState ps, OZ_Term * v)
{
  if (collect) {
    staticSuspendVars[staticSuspendVarsNumber].var = v;
    staticSuspendVars[staticSuspendVarsNumber].expected_type = OZ_VAR_FD;
    staticSuspendVars[staticSuspendVarsNumber++].state.fd = ps;

    staticSuspendVars.request(staticSuspendVarsNumber);
  }
}

// finite set constraints
inline
void OZ_Expect::addSuspend(OZ_FSetPropState ps, OZ_Term * v)
{
  if (collect) {
    staticSuspendVars[staticSuspendVarsNumber].var = v;
    staticSuspendVars[staticSuspendVarsNumber].expected_type = OZ_VAR_FS;
    staticSuspendVars[staticSuspendVarsNumber++].state.fs = ps;

    staticSuspendVars.request(staticSuspendVarsNumber);
  }
}

// generic constraints
inline
void OZ_Expect::addSuspend(OZ_CtDefinition * def, OZ_CtWakeUp w, OZ_Term * v)
{
  if (collect) {
    staticSuspendVars[staticSuspendVarsNumber].var = v;
    staticSuspendVars[staticSuspendVarsNumber].expected_type = OZ_VAR_CT;
    staticSuspendVars[staticSuspendVarsNumber].state.ct.def = def;
    staticSuspendVars[staticSuspendVarsNumber++].state.ct.w = w;

    staticSuspendVars.request(staticSuspendVarsNumber);
  }
}

//-----------------------------------------------------------------------------
// generic constraint variable

OZ_expect_t OZ_Expect::expectGenCtVar(OZ_Term t,
				      OZ_CtDefinition * def,
				      OZ_CtWakeUp w)
{
  DEREF(t, tptr);

  if (def->isValueOfDomain(t)) {
    return expectProceed(1, 1);
  } else if (isGenCtVar(t)) {
    OzCtVariable * ctvar = tagged2GenCtVar(t);

    // check the kind
    if (ctvar->getDefinition()->getId() != def->getId())
      goto fail;

    addSpawn(def, w, tptr);
    return expectProceed(1, 1);
  } else if (oz_isFree(t)) {
    addSuspend(def, w, tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(def, w, tptr);
    return expectExceptional();
  }

fail:
  return expectFail();
}



//*****************************************************************************
// member functions related to finite domain constraints
//*****************************************************************************

//-----------------------------------------------------------------------------
// An OZ term describing a finite domain is either:
// (1) a positive small integer <= FD.sup
// (2) a 2 tuple of (1)
// (3) a non-empty list of (1) and/or (2) or (1) or (2)
// (4) a tuple compl() of 4

OZ_expect_t OZ_Expect::expectDomDescr(OZ_Term descr, int level)
{
  DEREF(descr, descr_ptr);
  Assert(!oz_isRef(descr));

  if (level >= 4) {
    if (oz_isFree(descr)||oz_isKinded(descr)) {
      addSuspend(descr_ptr);
      return expectSuspend(1, 0);
    } else if (oz_isSTuple(descr) && tagged2SRecord(descr)->getWidth() == 1 &&
	       AtomCompl == tagged2SRecord(descr)->getLabel()) {
      //mm2: use tagged2Ref
      return expectDomDescr(makeTaggedRef(&(*tagged2SRecord(descr))[0]), 3);
    } else if (oz_isVarOrRef(descr)) {
      addSuspend(descr_ptr);
      return expectExceptional();
    }
    level = 3;
  }

  if (isPosSmallFDInt(descr) && (level >= 1)) { // (1)
    return expectProceed(1, 1);
  } else if (isGenFDVar(descr) && (level >= 1)) {
    addSuspend(fd_prop_singl, descr_ptr);
    return expectSuspend(1, 0);
  } else if (isGenBoolVar(descr) && (level >= 1)) {
    addSuspend(descr_ptr);
    return expectSuspend(1, 0);
  } else if (oz_isSTuple(descr) && (level >= 2)) {
    SRecord &tuple = *tagged2SRecord(descr);
    if (tuple.getWidth() != 2)
      return expectFail();
    for (int i = 0; i < 2; i++) {
      //mm2: use tagged2Ref
      OZ_expect_t r = expectDomDescr(makeTaggedRef(&tuple[i]), 1);
      if (isSuspending(r) || isFailing(r) || isExceptional(r))
	return r;
    }
    return expectProceed(1, 1);
  } else if (oz_isNil(descr) && (level == 3)) {
    return expectProceed(1, 1);
  } else if (oz_isLTupleOrRef(descr) && (level == 3)) {

    do {
      LTuple &list = *tagged2LTuple(descr);
      //mm2: use tagged2Ref
      OZ_expect_t r = expectDomDescr(makeTaggedRef(list.getRefHead()), 2);
      if (isSuspending(r) || isFailing(r) || isExceptional(r))
	return r;
      //mm2: use tagged2Ref
      descr = makeTaggedRef(list.getRefTail());

      _DEREF(descr, descr_ptr);
      Assert(!oz_isRef(descr));
    } while (oz_isLTuple(descr));

    if (oz_isNil(descr)) return expectProceed(1, 1);
    return expectDomDescr(makeTaggedRef(descr_ptr), 0);
  } else if (oz_isFree(descr)||oz_isKinded(descr)) {
    addSuspend(descr_ptr);
    return expectSuspend(1, 0);
  } else if (oz_isVarOrRef(descr)) {
    addSuspend(descr_ptr);
    return expectExceptional();
  }
  return expectFail();
}

// tmueller: not ready yet!
OZ_expect_t OZ_Expect::expectBoolVar(OZ_Term t)
{
  DEREF(t, tptr);

  if (isPosSmallBoolInt(t)) {
    return expectProceed(1, 1);
  } else if (isGenBoolVar(t)) {
    addSpawnBool(tptr);
    return expectProceed(1, 1);
  } else if (isGenFDVar(t)) {
    if (tellBasicBoolConstraint(makeTaggedRef(tptr)) == FAILED)
      return expectFail();
    addSpawnBool(tptr);
    return expectProceed(1, 1);
  } else if (oz_isFree(t) || oz_isKinded(t)) {
    addSuspendBool(tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspendBool(tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectIntVar(OZ_Term t, OZ_FDPropState ps)
{
  DEREF(t, tptr);

  if (isPosSmallFDInt(t)) {
    return expectProceed(1, 1);
  } else if (isGenBoolVar(t) || isGenFDVar(t)) {
    addSpawn(ps, tptr);
    return expectProceed(1, 1);
  } else if (oz_isFree(t) || oz_isKinded(t)) {
    addSuspend(ps, tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(ps, tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectInt(OZ_Term t)
{
  Assert(oz_isRef(t) || !oz_isVar(t));

  DEREF(t, tptr);

  if (oz_isSmallInt(t)) {
    return expectProceed(1, 1);
  } else if (oz_isFree(t)|| oz_isKinded(t)) {
    addSuspend(fd_prop_singl, tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(fd_prop_singl, tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectFloat(OZ_Term t)
{
  Assert(oz_isRef(t) || !oz_isVar(t));

  DEREF(t, tptr);

  if (oz_isFloat(t)) {
    return expectProceed(1, 1);
  } else if (oz_isFree(t)|| oz_isKinded(t)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(tptr);
    return expectExceptional();
  }
  return expectFail();
}

//*****************************************************************************
// member functions related to finite set constraints
//*****************************************************************************

//-----------------------------------------------------------------------------
// An OZ term describing a finite set is the same as for a finite domain

OZ_expect_t OZ_Expect::expectFSetDescr(OZ_Term descr, int level)
{
  return expectDomDescr(descr, level);
}

OZ_expect_t OZ_Expect::expectFSetVar(OZ_Term t, OZ_FSetPropState ps)
{
  DEREF(t, tptr);

  if (oz_isFSetValue(t)) {
    return expectProceed(1, 1);
  } else if (isGenFSetVar(t)) {
    addSpawn(ps, tptr);
    return expectProceed(1, 1);
  } else if (oz_isFree(t) || oz_isKinded(t)) {
    addSuspend(ps, tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(ps, tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectFSetValue(OZ_Term t)
{
  Assert(oz_isRef(t) || !oz_isVar(t));

  DEREF(t, tptr);

  if (oz_isFSetValue(t)) {
    return expectProceed(1, 1);
  } else if (oz_isFree(t)||oz_isKinded(t)) {
    addSuspend(fs_prop_val, tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(fs_prop_val, tptr);
    return expectExceptional();
  }
  return expectFail();
}

//*****************************************************************************

OZ_expect_t OZ_Expect::expectVar(OZ_Term t)
{
  DEREF(t, tptr);

  if (oz_isFree(t)) {
    addSpawn(fd_prop_any, tptr);
    return expectProceed(1, 1);
  } else if (oz_isNonKinded(t)) {
    addSuspend(tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectRecordVar(OZ_Term t)
{
  DEREF(t, tptr);

  if (oz_isRecord(t)) {
    return expectProceed(1, 1);
  } else if (isGenOFSVar(t)) {
    addSpawn(fd_prop_any, tptr);
    return expectProceed(1, 1);
  } else if (oz_isFree(t) || oz_isKinded(t)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectLiteral(OZ_Term t)
{
  Assert(oz_isRef(t) || !oz_isVar(t));

  DEREF(t, tptr);

  if (oz_isLiteral(t)) {
    return expectProceed(1, 1);
  } else if (oz_isFree(t) || oz_isKinded(t)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectLiteralOutOf(OZ_Term t, OZ_Term * one_of)
{
  OZ_expect_t r = expectLiteral(t);

  if (r.accepted == 1 && r.size == 1) {
    OZ_Term td = oz_deref(t);
    for (int i = 0; one_of[i] != (OZ_Term) 0; i += 1) {
      if (td == one_of[i])
	return expectProceed(1, 1);
    }
    return expectFail();
  }
  return r;
}

OZ_expect_t OZ_Expect::expectProperRecord(OZ_Term t,
					  OZ_expect_t(OZ_Expect::* expectf)(OZ_Term))
{
  Assert(oz_isRef(t) || !oz_isVar(t));

  DEREF(t, tptr);

  if (oz_isLiteral(t)) { // subsumes nil
    return expectProceed(1, 1);
  } else if (oz_isSRecord(t) && !tagged2SRecord(t)->isTuple()) {

    SRecord & tuple = *tagged2SRecord(t);
    int width = tuple.getWidth(), acc = 1;

    for (int i = width; i--; ) {
      //mm2: use tagged2Ref
      OZ_expect_t r = (this->*expectf)(makeTaggedRef(&tuple[i]));
      if (r.accepted == -1) {
	return r;
      } else if (r.accepted == r.size) {
	acc += 1;
      }
    }
    return expectProceed(width + 1, acc);

  } else if (oz_isFree(t) || oz_isKinded(t)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectProperRecord(OZ_Term t, OZ_Term * ar)
{
  Assert(oz_isRef(t) || !oz_isVar(t));

  DEREF(t, tptr);

  if (oz_isLiteral(t) && *ar == (OZ_Term) 0) { // subsumes nil
    return expectProceed(1, 1);
  } else if (oz_isSRecord(t) && !tagged2SRecord(t)->isTuple()) {
    int i;
    for (i = 0; ar[i] != (OZ_Term) 0; i += 1)
      if (OZ_subtree(t, ar[i]) == (OZ_Term) 0)
	return expectFail();

    return expectProceed(i + 1, i + 1);
  } else if (oz_isFree(t) || oz_isKinded(t)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectProperTuple(OZ_Term t,
					 OZ_expect_t(OZ_Expect::* expectf)(OZ_Term))
{
  Assert(oz_isRef(t) || !oz_isVar(t));

  DEREF(t, tptr);

  if (oz_isLiteral(t)) { // subsumes nil
    return expectProceed(1, 1);
  } else if (oz_isSRecord(t) && tagged2SRecord(t)->isTuple()) {

    SRecord & tuple = *tagged2SRecord(t);
    int width = tuple.getWidth(), acc = 1;

    for (int i = width; i--; ) {
      //mm2: use tagged2Ref
      OZ_expect_t r = (this->*expectf)(makeTaggedRef(&tuple[i]));
      if (r.accepted == -1) {
	return r;
      } else if (r.accepted == r.size) {
	acc += 1;
      }
    }
    return expectProceed(width + 1, acc);

  } else if (oz_isFree(t) || oz_isKinded(t)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectList(OZ_Term t,
				  OZ_expect_t(OZ_Expect::* expectf)(OZ_Term))
{
  Assert(oz_isRef(t) || !oz_isVar(t));

  DEREF(t, tptr);

  Assert(!oz_isRef(t));
  if (oz_isLTupleOrRef(t)) {

    int len = 0, acc = 0;

    do {
      len += 1;
      //mm2: use tagged2Ref
      OZ_expect_t r = (this->*expectf)(makeTaggedRef(tagged2LTuple(t)->getRefHead()));

      if (r.accepted == -1) {
	return r;
      } else if (r.accepted == r.size) {
	acc += 1;
      }

      t = oz_tail(t);
      _DEREF(t, tptr);
      Assert(!oz_isRef(t));
    } while (oz_isLTupleOrRef(t));

    if (oz_isNil(t)) {
      return expectProceed(len, acc);
    } else if (oz_isFree(t) || oz_isKinded(t)) {
      addSuspend(tptr);
      return expectSuspend(len+1, acc);
    } else if (oz_isNonKinded(t)) {
      addSuspend(tptr);
      return expectExceptional();
    }
  } else if (oz_isNil(t)) {
    return expectProceed(1, 1);
  } else if (oz_isFree(t) || oz_isKinded(t)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectVector(OZ_Term t,
				    OZ_expect_t(OZ_Expect::* expectf)(OZ_Term))
{
  Assert(oz_isRef(t) || !oz_isVar(t));

  DEREF(t, tptr);

  Assert(!oz_isRef(t));
  if (oz_isLiteral(t)) { // subsumes nil
    return expectProceed(1, 1);
  } else if (oz_isSTuple(t) || oz_isSRecord(t)) {

    SRecord & tuple = *tagged2SRecord(t);
    int width = tuple.getWidth(), acc = 1;

    for (int i = width; i--; ) {
      OZ_expect_t r = (this->*expectf)(makeTaggedRef(&tuple[i]));
      if (r.accepted == -1) {
	return r;
      } else if (r.accepted == r.size) {
	acc += 1;
      }
    }

#ifdef COUNT_PROP_INVOCS
    extern int count_prop_invocs_max_len_vec;
    count_prop_invocs_max_len_vec = max(count_prop_invocs_max_len_vec,
					width);
    extern int count_prop_invocs_min_len_vec;
    count_prop_invocs_min_len_vec = min(count_prop_invocs_min_len_vec,
					width);
    extern int count_prop_invocs_sum_len_vec;
    count_prop_invocs_sum_len_vec += width;
    extern int count_prop_invocs_nb_nonempty_vec;
    count_prop_invocs_nb_nonempty_vec += 1;
#endif

    return expectProceed(width + 1, acc);

  } else if (oz_isLTupleOrRef(t)) {

    int len = 0, acc = 0;

    do {
      len += 1;
      OZ_expect_t r = (this->*expectf)(makeTaggedRef(tagged2LTuple(t)->getRefHead()));

      if (r.accepted == -1) {
	return r;
      } else if (r.accepted == r.size) {
	acc += 1;
      }

      t = oz_tail(t);
      _DEREF(t, tptr);
      Assert(!oz_isRef(t));
    } while (oz_isLTupleOrRef(t));

    if (oz_isNil(t)) {

#ifdef COUNT_PROP_INVOCS
    extern int count_prop_invocs_max_len_vec;
    count_prop_invocs_max_len_vec = max(count_prop_invocs_max_len_vec,
					len);
    extern int count_prop_invocs_min_len_vec;
    count_prop_invocs_min_len_vec = min(count_prop_invocs_min_len_vec,
					len);
    extern int count_prop_invocs_sum_len_vec;
    count_prop_invocs_sum_len_vec += len;
    extern int count_prop_invocs_nb_nonempty_vec;
    count_prop_invocs_nb_nonempty_vec += 1;
#endif

      return expectProceed(len, acc);
    } else if (oz_isFree(t) || oz_isKinded(t)) {
      addSuspend(tptr);
      return expectSuspend(len+1, acc);
    } else if (oz_isNonKinded(t)) {
      addSuspend(tptr);
      return expectExceptional();
    }

  } else if (oz_isFree(t) || oz_isKinded(t)) {
    addSuspend(tptr);
    return expectSuspend(1, 0);
  } else if (oz_isNonKinded(t)) {
    addSuspend(tptr);
    return expectExceptional();
  }
  return expectFail();
}

OZ_expect_t OZ_Expect::expectStream(OZ_Term st)
{
  Assert(oz_isRef(st) || !oz_isVar(st));

  DEREF(st, stptr);

  Assert(!oz_isRef(st));
  if (oz_isNil(st)) {
    return expectProceed(1, 1);
  } else if (oz_isLTupleOrRef(st)) {

    int len = 0;

    do {
      len += 1;
      st = oz_tail(st);
      _DEREF(st, stptr);
      Assert(!oz_isRef(st));
      Assert(!oz_isRef(st));
    } while (oz_isLTupleOrRef(st));

    if (oz_isNil(st)) {
      return expectProceed(len, len);
    } else if (oz_isFree(st) || oz_isKinded(st)) {
      addSpawn(fd_prop_any, stptr);
      return expectProceed(len, len);
    } else if (oz_isNonKinded(st)) {
      addSuspend(stptr);
      return expectExceptional();
    }
  } else if (oz_isFree(st) || oz_isKinded(st)) {
    addSpawn(fd_prop_any, stptr);
    return expectProceed(1, 1);
  } else if (oz_isNonKinded(st)) {
    addSuspend(stptr);
    return expectExceptional();
  }

  return expectFail();
}

//-----------------------------------------------------------------------------

OZ_Return OZ_Expect::suspend(void)
{
  Assert(staticSuspendVarsNumber > 0);

  for (int i = staticSuspendVarsNumber; i--; )
    (void) am.addSuspendVarListInline(staticSuspendVars[i].var);
  return SUSPEND;
}

OZ_Return OZ_Expect::fail(void)
{
  return FAILED;
}


//*****************************************************************************
// member function to spawn a propagator
//*****************************************************************************

#ifdef COUNT_PROP_INVOCS
int count_prop_invocs_created         = 0;
int count_prop_invocs_run             = 0;
int count_prop_invocs_sleep           = 0;
int count_prop_invocs_fail            = 0;
int count_prop_invocs_entail          = 0;

int count_prop_invocs_local_params    = 0;
int count_prop_invocs_global_params   = 0;
int count_prop_invocs_det_params      = 0;

int count_prop_invocs_max_runnable    = 0;
int count_prop_invocs_min_runnable    = INT_MAX;
double count_prop_invocs_sum_runnable = 0.0;
int count_prop_invocs_nb_smp_runnable = 0;

int count_prop_invocs_max_len_el      = 0;
int count_prop_invocs_min_len_el      = INT_MAX;
int count_prop_invocs_sum_len_el      = 0;
int count_prop_invocs_nb_nonempty_el  = 0;

int count_prop_invocs_max_len_sl      = 0;
int count_prop_invocs_min_len_sl      = INT_MAX;
int count_prop_invocs_sum_len_sl      = 0;
int count_prop_invocs_nb_nonempty_sl  = 0;

int count_prop_invocs_max_len_vec     = 0;
int count_prop_invocs_min_len_vec     = INT_MAX;
int count_prop_invocs_sum_len_vec     = 0;
int count_prop_invocs_nb_nonempty_vec = 0;

int count_prop_invocs_fdvars_created   = 0;

#endif 

Propagator * imposed_propagator;
int is_active = 1;

int OZ_CPIVar::_first_run = 0;
OZ_Term OZ_CPIVar::_vars_removed;

OZ_Return OZ_Expect::impose(OZ_Propagator * p)
{
  OZ_Boolean is_monotonic = p->isMonotonic();
  OZ_Return ret = PROCEED;

#ifdef COUNT_PROP_INVOCS
      count_prop_invocs_created += 1;
    count_prop_invocs_sum_runnable += 1;
    count_prop_invocs_nb_smp_runnable += 1;
#endif


  // do initial run with dummy thread

  // Constrain all SimpleVar"s and OptVar"s in staticSuspendVars to
  // FDVARs before OZ_Propagator::run is run.
  int i;
  for (i = staticSuspendVarsNumber; i--; ) {
    OZ_Term v = makeTaggedRef(staticSuspendVars[i].var);
    DEREF(v, vptr);
    TypeOfVariable type = staticSuspendVars[i].expected_type;

    Assert(type == OZ_VAR_FD || type == OZ_VAR_FS ||
	   type == OZ_VAR_CT || type == OZ_VAR_BOOL);

    if (oz_isFree(v)) {
      if (type == OZ_VAR_FD) {
	tellBasicConstraint(makeTaggedRef(vptr), (OZ_FiniteDomain *) NULL);
      } else if (type == OZ_VAR_BOOL) {
	tellBasicBoolConstraint(makeTaggedRef(vptr));
      } else if (type == OZ_VAR_FS) {
	tellBasicConstraint(makeTaggedRef(vptr), (OZ_FSetConstraint *) NULL);
      } else {
	Assert(type == OZ_VAR_CT);

	tellBasicConstraint(makeTaggedRef(vptr),
			    (OZ_Ct *) NULL,
			    staticSuspendVars[i].state.ct.def);

      }
    } else {
      Assert(oz_isKinded(v));
    }
  }

  Propagator * prop = imposed_propagator = oz_newPropagator(p);

  ozstat.propagatorsCreated.incf();

#ifdef NAME_PROPAGATORS
  if (am.isPropagatorLocation()) {
    Thread * thr = oz_currentThread();
    OZ_Term debug_frame_raw =
      thr->getTaskStackRef()->getTaskStack(thr, TRUE, 1);

    OZ_Term abstr_stack_entry =
      thr->getTaskStackRef()->findAbstrRecord();

    if (debug_frame_raw != oz_nil()) {
      OZ_Term debug_frame = OZ_head(debug_frame_raw);

      const char * fname = OZ_atomToC(OZ_subtree(debug_frame, AtomFile));
      char * dirname, * basename;

      splitfname(fname, dirname, basename);

      OZ_Term prop_loc
	= OZ_record(AtomPropLocation,
		    OZ_cons(AtomPropInvoc,
			    OZ_cons(AtomFile,
				    OZ_cons(AtomLine,
					    OZ_cons(AtomColumn,
						    OZ_cons(AtomPath,
							    oz_nil()))))));
      OZ_putSubtree(prop_loc, AtomPath, OZ_atom(dirname));
      OZ_putSubtree(prop_loc, AtomFile, OZ_atom(basename));
      OZ_putSubtree(prop_loc, AtomLine, OZ_subtree(debug_frame, AtomLine));
      OZ_putSubtree(prop_loc, AtomColumn, OZ_subtree(debug_frame, AtomColumn));
      OZ_putSubtree(prop_loc, AtomPropInvoc, abstr_stack_entry);

      oz_propAddName(prop, prop_loc);

      NEW_NAMER_DEBUG_PRINT(("added propLoc for = %p (%s)\n",
			     imposed_propagator, OZ_toC(prop_loc, 100, 100)));
    }
  }
#endif

  // only monotonic propagator are run on imposition
  if (is_monotonic) {
    ozstat.propagatorsInvoked.incf();

    Propagator::setRunningPropagator(prop);

#ifdef COUNT_PROP_INVOCS
    for (i = staticSpawnVarsNumber; i--; ) {
      OZ_Term v = makeTaggedRef(staticSpawnVars[i].var);
      DEREF(v, vptr);
      if (oz_isVar(v)) {
	if (oz_isLocalVar(tagged2Var(v))) {
	  count_prop_invocs_local_params += 1;
	} else {
	  count_prop_invocs_global_params += 1;
	}
      } else {
	count_prop_invocs_det_params += 1;
      }
    }
#endif

    // if a propagator is to be imposed `inactive' set the appropriate
    // flag and restore the `isactive' variable. `oz_runPropagator'
    // takes care of the rest.
    if (!is_active) {
      prop->unsetActive();
      is_active = 1;
    }
    OZ_CPIVar::set_vars_removed();
    switch (oz_runPropagator(prop)) {
    case OZ_FAILED:

#ifdef COUNT_PROP_INVOCS
      count_prop_invocs_fail += 1;
#endif

#ifdef NAME_PROPAGATORS
      // this is experimental: a top-level failure with set
      // property 'internal.propLocation',
      if (am.isPropagatorLocation()) {
	if (!am.hf_raise_failure()) {
	  oz_sleepPropagator(prop);
	  prop->setFailed();
	  ret = FAILED;
	  break;
	}
      }
#endif

      oz_closeDonePropagator(prop);
      staticSpawnVarsNumber = staticSuspendVarsNumber = 0;
      OZ_CPIVar::reset_vars_removed();
      return FAILED;
    case OZ_SLEEP:
#ifdef COUNT_PROP_INVOCS
      count_prop_invocs_sleep += 1;
#endif
      oz_sleepPropagator(prop);
      break;
    case SCHEDULED:
      oz_preemptedPropagator(prop);
      break;
    case OZ_ENTAILED:
#ifdef COUNT_PROP_INVOCS
      count_prop_invocs_entail += 1;
#endif
      oz_closeDonePropagator(prop);
      staticSpawnVarsNumber = staticSuspendVarsNumber = 0;
      OZ_CPIVar::reset_vars_removed();
      return PROCEED;
    default:
      DebugCode(OZ_error("Unexpected return value."));
      OZ_CPIVar::reset_vars_removed();
      return PROCEED;
    }
  }
  OZ_CPIVar::reset_vars_removed();

  // only if a propagator survives its first run proper suspension are created
  OZ_Boolean all_local = OZ_TRUE;

  // caused by the first run of the run method variables may have
  // become determined
  for (i = staticSpawnVarsNumber; i--; ) {
    OZ_Term v = makeTaggedRef(staticSpawnVars[i].var);
    DEREF(v, vptr);

    Assert(!oz_isRef(v));
    if (oz_isVarOrRef(v)) {

      if (OZ_CPIVar::is_in_vars_removed(vptr)) {
#ifdef DEBUG_REMOVE_PARAMS
	printf("continuing\n");
#endif
	continue;
      }
      if (isGenFDVar(v)) {
	if (oz_isLocalVar(tagged2Var(v))) {
	  addSuspFDVar(v, prop, staticSpawnVars[i].state.fd);
	  continue;
	}
      } else if (isGenFSetVar(v)) {
	if (oz_isLocalVar(tagged2Var(v))) {
	  addSuspFSetVar(v, prop, staticSpawnVars[i].state.fs);
	  continue;
	}
      } else if (isGenBoolVar(v)) {
	if (oz_isLocalVar(tagged2Var(v))) {
	  addSuspBoolVar(v, prop);
	  continue;
	}
      } else if (isGenCtVar(v)) {
	if (oz_isLocalVar(tagged2Var(v))) {
	  addSuspCtVar(v, prop, staticSpawnVars[i].state.ct.w);
	  continue;
	}
      }
      //    
      oz_var_addSusp(vptr, prop);
      all_local &= oz_isLocalVar(tagged2Var(*vptr));
      //
    }
  }
  //
  // Note all SimpleVar"s and OptVar"s in staticSuspendVars are
  // constrained to FDVARs.
  for (i = staticSuspendVarsNumber; i--; ) {
    OZ_Term v = makeTaggedRef(staticSuspendVars[i].var);
    DEREF(v, vptr);

    Assert(!oz_isRef(v));
    if (oz_isVarOrRef(v)) {
      oz_var_addSusp(vptr, prop);
      all_local &= oz_isLocalVar(tagged2Var(v));
    }
  }

  if (all_local)
    prop->setLocal();

  staticSpawnVarsNumber = staticSuspendVarsNumber = 0;

  // only nonmonotonic propagator are set runnable on imposition
  if (! is_monotonic) {
#ifdef DEBUG_NONMONOTONIC
    printf("Setting nonmono prop runnable.\n"); fflush(stdout);
#endif
    oz_preemptedPropagator(prop);
  }
  return ret;
}

// End of File
//-----------------------------------------------------------------------------
