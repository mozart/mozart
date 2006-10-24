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
  return oz_freeListMalloc(s);
}

void OZ_Propagator::operator delete(void * p, size_t s)
{
  oz_freeListDisposeUnsafe(p, s);
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
    DEREF(h, hptr);
    switch (tagged2ltag(h)) {

    case LTAG_LITERAL:
      o << tagged2Literal(h)->getPrintName();
      break;

    case LTAG_LTUPLE0:
    case LTAG_LTUPLE1:
      outputArgsList(o, h, TRUE);
      break;

    case LTAG_SRECORD0:
    case LTAG_SRECORD1:
      {
	SRecord * st = tagged2SRecord(h);
	if (st->isTuple()) {
	  int width = st->getWidth();
	  o << tagged2Literal(st->getLabel())->getPrintName() << '/' << width;
	}
      }
      break;

    case LTAG_CONST0:
    case LTAG_CONST1:
      if (oz_isFSetValue(h))
	o << tagged2FSetValue(h)->toString();
      if (oz_isFloat(h))
	o << floatValue(h);
      break;

    case LTAG_SMALLINT:
      o << tagged2SmallInt(h);
      break;

    case LTAG_VAR0:
    case LTAG_VAR1:
      {
	o << oz_varGetName(makeTaggedRef(hptr));
	
	OzVariable * cv = tagged2Var(h);
	
	switch (cv->getTypeMasked()) {
	case OZ_VAR_OPT:
	  o << '_';
	  break; 
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
	     "of propagator while printing %x, %x.", args, tagged2stag(args));
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
  DEREF(t, tptr);
  Assert(!oz_isRef(t));
  if (oz_isVarOrRef(t)) {
    oz_var_addSusp(tptr, Propagator::getRunningPropagator());
    return OZ_TRUE;
  } 
  return OZ_FALSE;
}

OZ_Boolean OZ_Propagator::addImpose(OZ_FDPropState ps, OZ_Term v)
{
  DEREF(v, vptr);
  Assert(!oz_isRef(v));
  if (!oz_isVarOrRef(v))
    return FALSE;
  Assert(vptr);

  staticAddSpawnProp(ps, vptr);
  return TRUE;
}

OZ_Boolean OZ_Propagator::addImpose(OZ_FSetPropState s, OZ_Term v)
{
  DEREF(v, vptr);
  Assert(!oz_isRef(v));
  if (!oz_isVarOrRef(v))
    return FALSE;
  Assert(vptr);

  staticAddSpawnProp(s, vptr);
  return TRUE;
}


OZ_Boolean OZ_Propagator::addImpose(OZ_CtWakeUp e,
				    OZ_CtDefinition * d,
				    OZ_Term v)
{
  DEREF(v, vptr);
  Assert(!oz_isRef(v));
  if (!oz_isVarOrRef(v))
    return FALSE;
  Assert(vptr);

  staticAddSpawnProp(d, e, vptr);
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
    DEREF(v, vptr);
    //
    Assert(oz_isVar(v));
    //
    void * cpi_raw = (void *) NULL;
    int isNonEncapTagged = 0, isEncapTagged = 0;
    //
    // kost@ : conditional subsumed by the assertion above;
    // if (oz_isVar(v)) {
      OzVariable * var = tagged2Var(v);
      isNonEncapTagged  = var->isParamNonEncapTagged();
      isEncapTagged     = var->isParamEncapTagged();
      cpi_raw           = var->getRawAndUntag();
    // }
    //
    if (isGenFDVar(v)) {
      addSuspFDVar(v, prop, staticSpawnVarsProp[i].state.fd);
      all_local &= oz_isLocalVar(tagged2Var(v));
    } else if (isGenOFSVar(v)) {
      addSuspOFSVar(v, prop);
      all_local &= oz_isLocalVar(tagged2Var(v));
    } else if (isGenBoolVar(v)) {
      addSuspBoolVar(v, prop);  
      all_local &= oz_isLocalVar(tagged2Var(v));
      // mm2:
      //    } else if (isSVar(vtag)) {
      //      addSuspSVar(v, prop);
      //      all_local &= isLocalVar(v);
    } else {
      Assert(oz_isOptVar(v));
      oz_var_addSusp(vptr, prop);
      all_local &= oz_isLocalVar(tagged2Var(*vptr));
    }
    //
    // undo everything
    //
    Assert(!oz_isRef(v));
    if (oz_isVar(v)) {
      OzVariable * var = tagged2Var(v);
      if (isNonEncapTagged) {
	var->setStoreFlag();
      }
      if (isEncapTagged) {
	var->setReifiedFlag();
      }
      var->putRawTag(cpi_raw);
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
