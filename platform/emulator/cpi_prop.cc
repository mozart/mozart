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
#include "var_fd.hh"
#include "var_bool.hh"
#include "var_fs.hh"
#include "var_ct.hh"
#include "var_of.hh"
#include "prop_int.hh"

// gc.cc: OZ_Propagator * OZ_Propagator::gc(void)

// propagators use free list heap memory
void * OZ_Propagator::operator new(size_t s)
{
  return freeListMalloc(s);
}

void OZ_Propagator::operator delete(void * p, size_t s)
{
  freeListDispose(p, s);
}

static void outputArgsList(ostream& o, OZ_Term args, Bool not_top)
{
  Bool not_first = FALSE;
  if (not_top) o << '[';

  for (; OZ_isCons(args); args = OZ_tail(args)) {
    OZ_Term h = OZ_head(args);
    if (not_first)
      o << ' ';
    not_first = 1;
    //
    DEREF(h, hptr, htag);
    switch (htag) {

    case TAG_FLOAT:
      o << floatValue(h);
      break;

    case TAG_LITERAL:
      o << tagged2Literal(h)->getPrintName();
      break;

    case TAG_LTUPLE:
      outputArgsList(o, h, TRUE);
      break;

    case TAG_SRECORD:
      {
        SRecord * st = tagged2SRecord(h);
        if (st->isTuple()) {
          int width = st->getWidth();
          o << tagged2Literal(st->getLabel())->getPrintName() << '/' << width;
        }
      }
      break;
    case TAG_UVAR:
      o << '_';
      break;

    case TAG_SMALLINT:
      o << tagged2SmallInt(h);
      break;

    case TAG_FSETVALUE:
      o << tagged2FSetValue(h)->toString();
      break;

    case TAG_CVAR:
      {
        o << oz_varGetName(makeTaggedRef(hptr));

        OzVariable * cv = tagged2CVar(h);

        switch (cv->getTypeMasked()) {
        case OZ_VAR_FD:
          o << ((OzFDVariable *) cv)->getDom().toString();
          break;
        case OZ_VAR_BOOL:
          o << "{0#1}";
          break;
        case OZ_VAR_FS:
          o << ((OzFSVariable *) cv)->getSet().toString();
          break;
        case OZ_VAR_CT:
          o << ((OzCtVariable *) cv)->getConstraint()->toString(0);
          break;
        default:
          goto problem;
        }
      }

      not_first = TRUE;
    default:
      break;
    }
  }
  if (!OZ_isNil(args))
    goto problem;
  if (not_top)
    o << ']' << flush;
  return;

 problem:
  OZ_warning("Unexpected term found in argument list "
             "of propagator while printing %x, %x.", args, tagTypeOf(args));
}

ostream& operator << (ostream& o, const OZ_Propagator &p)
{
  const char * func_name = p.getProfile()->getPropagatorName();
  OZ_Term args = p.getParameters();

  if (!p.isMonotonic())
    o << p.getOrder() << '#' << flush;

  o << '{' << func_name << ' ';
  outputArgsList(o, args, FALSE);
  o << '}' << flush;

  return o;
}

char *OZ_Propagator::toString() const
{
  ozstrstream str;
  str << (*this);
  return strdup(str.str());
}


OZ_Boolean OZ_Propagator::mayBeEqualVars(void)
{
  return Propagator::getRunningPropagator()->isUnify();
}


OZ_Return OZ_Propagator::replaceBy(OZ_Propagator * p)
{
  Propagator::getRunningPropagator()->setPropagator(p);
  return oz_runPropagator(Propagator::getRunningPropagator());
}

OZ_Return OZ_Propagator::replaceBy(OZ_Term a, OZ_Term b)
{
  return OZ_unify(a, b); // mm_u
}

OZ_Return OZ_Propagator::replaceByInt(OZ_Term v, int i)
{
  return OZ_unify(v, makeTaggedSmallInt(i)); // mm_u
}

OZ_Return OZ_Propagator::postpone(void)
{
  return SCHEDULED;
}

OZ_Boolean OZ_Propagator::imposeOn(OZ_Term t)
{
  DEREF(t, tptr, ttag);
  if (isVariableTag(ttag)) {
    oz_var_addSusp(tptr, Propagator::getRunningPropagator());
    return OZ_TRUE;
  }
  return OZ_FALSE;
}

OZ_Boolean OZ_Propagator::addImpose(OZ_FDPropState ps, OZ_Term v)
{
  DEREF(v, vptr, vtag);
  if (!isVariableTag(vtag))
    return FALSE;
  Assert(vptr);

  staticAddSpawnProp(ps, vptr);
  return TRUE;
}

int OZ_Propagator::impose(OZ_Propagator * p)
{
  Propagator * prop = oz_newPropagator(p);
  ozstat.propagatorsCreated.incf();

  oz_sleepPropagator(prop);

  prop->setRunnable();
  oz_currentBoard()->addToLPQ(prop);

  OZ_Boolean all_local = OZ_TRUE;

  for (int i = staticSpawnVarsNumberProp; i--; ) {
    OZ_Term v = makeTaggedRef(staticSpawnVarsProp[i].var);
    DEREF(v, vptr, _vtag);
    //
    Assert(oz_isVariable(v));
    //
    void * cpi_raw = (void *) NULL;
    int isNonEncapTagged = 0, isEncapTagged = 0;
    //
    if (oz_isCVar(v)) {
      OzVariable * cvar = tagged2CVar(v);
      isNonEncapTagged  = cvar->isParamNonEncapTagged();
      isEncapTagged     = cvar->isParamEncapTagged();
      cpi_raw           = cvar->getRawAndUntag();
    }
    //
    if (isGenFDVar(v)) {
      addSuspFDVar(v, prop, staticSpawnVarsProp[i].state.fd);
      all_local &= oz_isLocalVar(tagged2CVar(v));
    } else if (isGenOFSVar(v)) {
      addSuspOFSVar(v, prop);
      all_local &= oz_isLocalVar(tagged2CVar(v));
    } else if (isGenBoolVar(v)) {
      addSuspBoolVar(v, prop);
      all_local &= oz_isLocalVar(tagged2CVar(v));
      // mm2:
      //    } else if (isSVar(vtag)) {
      //      addSuspSVar(v, prop);
      //      all_local &= isLocalVar(v);
    } else {
      Assert(oz_isUVar(v));
      oz_var_addSusp(vptr, prop);
      all_local &= oz_isLocalVar(tagged2CVar(*vptr));
    }
    //
    // undo everything
    //
    if (oz_isCVar(v)) {
      OzVariable * cvar = tagged2CVar(v);
      if (isNonEncapTagged) {
        cvar->setStoreFlag();
      }
      if (isEncapTagged) {
        cvar->setReifiedFlag();
      }
      cvar->putRawTag(cpi_raw);
    }
  } // for
  //
  if (all_local)
    prop->setLocal();
  //
  staticSpawnVarsNumberProp = 0;
  //
  return 0;
}

//-----------------------------------------------------------------------------
// class NonMonotonic

OZ_NonMonotonic::order_t OZ_NonMonotonic::_next_order = 1;

OZ_NonMonotonic::OZ_NonMonotonic(void) : _order(_next_order++)
{
  Assert(_next_order);

#ifdef DEBUG_NONMONOTONIC
    printf("New nonmono order: %d\n", _next_order-1); fflush(stdout);
#endif

}

// End of File
//-----------------------------------------------------------------------------
