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
 *     http://mozart.ps.uni-sb.de
 * 
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
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
    if (not_first) o << ' ';

    DEREF(h, hptr, htag);
    switch (htag) {

    case OZFLOAT:
      o << floatValue(h);
      break;

    case LITERAL:
      o << tagged2Literal(h)->getPrintName();
      break;

    case LTUPLE:
      outputArgsList(o, h, TRUE);
      break;

    case SRECORD:
      {
	SRecord * st = tagged2SRecord(h);
	if (st->isTuple()) {
	  int width = st->getWidth();
	  o << tagged2Literal(st->getLabel())->getPrintName() << '/' << width;
	}
      }
      break;
    case UVAR: // FUT
      o << '_';
      break; 

    case SMALLINT:
      o << smallIntValue(h);
      break;

    case FSETVALUE:
      o << tagged2FSetValue(h)->toString();
      break;

    case CVAR:
      {
	o << oz_varGetName(makeTaggedRef(hptr));

	OzVariable * cv = tagged2CVar(h);
	
	if (cv->testReifiedFlag()) {

	  if (cv->isBoolPatched()) 
	    goto bool_lbl; 
	  if (cv->isFDPatched()) 
	    goto fd_lbl;
	  if (cv->isFSetPatched()) 
	    goto fs_lbl;
	  if (cv->isCtPatched()) 
	    goto ct_lbl; 

	} else if (cv->getType() == OZ_VAR_FD) {
	fd_lbl:
	  o << ((OzFDVariable *) cv)->getDom().toString();
	} else if (cv->getType() == OZ_VAR_BOOL) {
	bool_lbl:
	  o << "{0#1}";
	} else if (cv->getType() == OZ_VAR_FS) {
	fs_lbl:
	  o << ((OzFSVariable *) cv)->getSet().toString();
	} else if (cv->getType() == OZ_VAR_CT) {
	ct_lbl:
	  o << ((OzCtVariable *) cv)->getConstraint()->toString(0);
	} else {	  
	  goto problem;
	}
      }
      break;

    default:
      goto problem;
    }
    
    not_first = TRUE;
  }

  if (!OZ_isNil(args)) goto problem;
  if (not_top) o << ']' << flush;
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
  return ozstrdup(str.str());
}


OZ_Boolean OZ_Propagator::mayBeEqualVars(void)
{
  return Propagator::getRunningPropagator()->isUnifyPropagator();
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
  return OZ_unify(v, newSmallInt(i)); // mm_u
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

void OZ_Propagator::impose(OZ_Propagator * p) 
{
  Propagator * prop = oz_newPropagator(p);
  ozstat.propagatorsCreated.incf();

  oz_sleepPropagator(prop);

  prop->markRunnable();
  oz_pushToLPQ(oz_currentBoard(),prop);

  OZ_Boolean all_local = OZ_TRUE;

  for (int i = staticSpawnVarsNumberProp; i--; ) {
    OZ_Term v = makeTaggedRef(staticSpawnVarsProp[i].var);
    DEREF(v, vptr, vtag);

    Assert(isVariableTag(vtag));

    Bool isStorePatched = 0, isReifiedPatched = 0, isBoolPatched = 0;
    OZ_FiniteDomain * tmp_fd = NULL;

    // TMUELLER: needs to extended to FSet and GenConstraint
    if (isCVar(vtag)) {
      isStorePatched = testResetStoreFlag(v);
      isReifiedPatched = testResetReifiedFlag(v);
      if (isReifiedPatched) {
	isBoolPatched = testBoolPatched(v);
	tmp_fd = unpatchReifiedFD(v, isBoolPatched);
      }
    }

    if (isGenFDVar(v, vtag)) {
      addSuspFDVar(v, prop, staticSpawnVarsProp[i].state.fd);
      all_local &= oz_isLocalVar(tagged2CVar(v));
    } else if (isGenOFSVar(v, vtag)) {
      addSuspOFSVar(v, prop);
      all_local &= oz_isLocalVar(tagged2CVar(v));
    } else if (isGenBoolVar(v, vtag)) {
      addSuspBoolVar(v, prop);  
      all_local &= oz_isLocalVar(tagged2CVar(v));
      // mm2:
      //    } else if (isSVar(vtag)) {
      //      addSuspSVar(v, prop);
      //      all_local &= isLocalVar(v);
    } else {
      Assert(isUVar(vtag));
      oz_var_addSusp(vptr, prop);
      all_local &= oz_isLocalVar(tagged2CVar(*vptr));
    }

    if (isCVar(vtag)) {
      if (isStorePatched) 
	setStoreFlag(v);
      if (isReifiedPatched)
	patchReified(tmp_fd, v, isBoolPatched);
    }
  }

  if (all_local) 
    prop->markLocalPropagator();
  
  staticSpawnVarsNumberProp = 0;
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
