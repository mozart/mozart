/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#include "am.hh"
#include "cpi.hh"
#include "fdproto.hh"

//*****************************************************************************

OZ_C_proc_begin(BIfsIsVarB, 2)
{
  return OZ_unify(OZ_getCArg (1),
                  (isGenFSetVar(deref(OZ_getCArg(0))) ? NameTrue : NameFalse));
}
OZ_C_proc_end


OZ_C_proc_begin(BIfsSetValue, 2)
{
  ExpectedTypes(OZ_EM_FSETDESCR "," OZ_EM_FSETVAL);

  ExpectOnly pe;

  EXPECT_BLOCK(pe, 0, expectFSetDescr);

  return OZ_unify(OZ_getCArg(1),
                  makeTaggedFSetValue(new FSetValue(OZ_getCArg(0))));
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsSet, 3)
{
  ExpectedTypes(OZ_EM_FSETDESCR "," OZ_EM_FSETDESCR "," OZ_EM_FSET);


  ExpectOnly pe;

  EXPECT_BLOCK(pe, 0, expectFSetDescr);
  EXPECT_BLOCK(pe, 1, expectFSetDescr);

  OZ_FSetImpl fset(OZ_getCArg(0), OZ_getCArg(1));

  if (! fset.isValid()) {
    TypeError(2, "Invalid set description");
    return FAILED;
  }

  return tellBasicConstraint(OZ_getCArg(2), &fset);
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsSup, 1)
{
  return OZ_unify(OZ_getCArg(0), OZ_int(fset_sup));
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(BIfsGetKnownIn, 2)
{
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValue(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), fsetval->getKnownInList());
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), fsetconstr->getKnownInList());
  } else if (isNotCVar(vtag)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }
    return FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsGetNumOfKnownIn, 2)
{
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValue(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetval->getCard()));
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetconstr->getKnownIn()));
  } else if (isNotCVar(vtag)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }
    return FAILED;
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(BIfsGetKnownNotIn, 2)
{
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValue(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), fsetval->getKnownNotInList());
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), fsetconstr->getKnownNotInList());
  } else if (isNotCVar(vtag)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }
    return FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsGetNumOfKnownNotIn, 2)
{
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValue(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetval->getKnownNotIn()));
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetconstr->getKnownNotIn()));
  } else if (isNotCVar(vtag)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }
    return FAILED;
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(BIfsGetUnknown, 2)
{
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValue(vtag)) {
    return OZ_unify(OZ_getCArg(1), OZ_nil());
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), fsetconstr->getUnknownList());
  } else if (isNotCVar(vtag)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }
    return FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsGetNumOfUnknown, 2)
{
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValue(vtag)) {
    return OZ_unify(OZ_getCArg(1), OZ_int(0));
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetconstr->getUnknown()));
  } else if (isNotCVar(vtag)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }
    return FAILED;
}
OZ_C_proc_end

//-----------------------------------------------------------------------------

OZ_C_proc_begin(BIfsGetLub, 2)
{
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValue(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), fsetval->getKnownInList());
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), fsetconstr->getLubList());
  } else if (isNotCVar(vtag)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }
    return FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsGetCard, 2)
{
  OZ_Term v = OZ_getCArg(0);
  DEREF(v, vptr, vtag);

  if (isFSetValue(vtag)) {
    OZ_FSetValue * fsetval = tagged2FSetValue(v);
    return OZ_unify(OZ_getCArg(1), OZ_int(fsetval->getCard()));
  } else if (isGenFSetVar(v, vtag)) {
    OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
    return OZ_unify(OZ_getCArg(1), fsetconstr->getCardTuple());
  } else if (isNotCVar(vtag)) {
    return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
  }
    return FAILED;
}
OZ_C_proc_end

OZ_C_proc_begin(BIfsCardRange, 3)
{
  int l = -1;
  {
    OZ_Term lt = OZ_getCArg(0);
    DEREF(lt, ltptr, lttag);

    if (isSmallInt(lttag)) {
      l = smallIntValue(lt);
    } else if (isAnyVar(lttag)) {
      return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, ltptr);
    } else {
      return FAILED;
    }
  }

  int u = -1;
  {
    OZ_Term ut = OZ_getCArg(1);
    DEREF(ut, utptr, uttag);

    if (isSmallInt(uttag)) {
      u = smallIntValue(ut);
    } else if (isAnyVar(uttag)) {
      return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, utptr);
    } else {
      return FAILED;
    }
  }

  if (l > u)
    return FAILED;

  {
    OZ_Term v = OZ_getCArg(2);
    DEREF(v, vptr, vtag);

    if (isFSetValue(vtag)) {
      int card = tagged2FSetValue(v)->getCard();
      return ((l <= card) && (card <= u)) ? PROCEED : FAILED;
    } else if (isGenFSetVar(v, vtag)) {
      OZ_FSetConstraint * fsetconstr = &tagged2GenFSetVar(v)->getSet();
      if (!fsetconstr->putCard(l, u))
        return FAILED;
      /* a variable might have become s fset value because of
         imposing a card constraints */
      if (fsetconstr->isValue())
        tagged2GenFSetVar(v)->becomesFSetValueAndPropagate(vptr);
      return PROCEED;
    } else if (isNotCVar(vtag)) {
      return constraintsSuspendOnVar(OZ_self, OZ_arity, OZ_args, vptr);
    }
  }
  return FAILED;
}
OZ_C_proc_end

// TMUELLER: already redundant
OZ_C_proc_begin(BImkFSetVar, 5)
{
  OZ_FSetImpl fset(OZ_intToC(OZ_getCArg(0)), OZ_intToC(OZ_getCArg(1)),
                   OZ_getCArg(2), OZ_getCArg(3));

  return OZ_unify(OZ_getCArg(4), makeTaggedRef(newTaggedCVar(new GenFSetVariable(fset))));
}
OZ_C_proc_end

static
BIspec fdSpec[] = {

// fsetcore.cc
  {"fsIsVarB", 2, BIfsIsVarB},
  {"fsSetValue", 2, BIfsSetValue},
  {"fsSet", 3, BIfsSet},
  {"fsSup", 1, BIfsSup},
  {"fsGetKnownIn", 2, BIfsGetKnownIn},
  {"fsGetKnownNotIn", 2, BIfsGetKnownNotIn},
  {"fsGetUnknown", 2, BIfsGetUnknown},
  {"fsGetGlb", 2, BIfsGetKnownIn},
  {"fsGetLub", 2, BIfsGetLub},
  {"fsGetCard", 2, BIfsGetCard},
  {"fsCardRange", 3, BIfsCardRange},
  {"fsGetNumOfKnownIn", 2, BIfsGetNumOfKnownIn},
  {"fsGetNumOfKnownNotIn", 2, BIfsGetNumOfKnownNotIn},
  {"fsGetNumOfUnknown", 2, BIfsGetNumOfUnknown},

  {"mkFSetVar", 5, BImkFSetVar},

#ifndef FOREIGNFDPROPS
  {"fsp_init",         0, fsp_init},
  {"fsp_isIn",         3, fsp_isIn},
  {"fsp_include",      2, fsp_include},
  {"fsp_exclude",      2, fsp_exclude},
  {"fsp_card",         2, fsp_card},
  {"fsp_union",        3, fsp_union},
  {"fsp_intersection", 3, fsp_intersection},
  {"fsp_subsume",      2, fsp_subsume},
  {"fsp_disjoint",     2, fsp_disjoint},
  {"fsp_distinct",     2, fsp_distinct},
  {"fsp_monitorIn",    2, fsp_monitorIn},
  {"fsp_min",          2, fsp_min},
  {"fsp_max",          2, fsp_max},
  {"fsp_convex",       1, fsp_convex},
  {"fsp_diff",         3, fsp_diff},
  {"fsp_includeR",     3, fsp_includeR},
#endif /* FOREIGNFDPROPS */

  {0,0,0,0}
};

void BIinitFSet(void)
{
  BIaddSpec(fdSpec);
}
