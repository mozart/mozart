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

#include "reflect.hh"

//=============================================================================

#define EXPECT_PROPGATORREF \
OZ_atom("Expecting Propagator Reference")

#define EXPECT_PROPGATORDISCARDED \
OZ_atom("Propagator Reference is DISCARDED")

#define EXCEPTION \
"reflect"

//-----------------------------------------------------------------------------

OZ_BI_define(BIIdentifyParameter, 1, 1)
{
  DEBUGPRINT(("BIIdentifyParameter in\n"));

  ExpectedTypes("List of variables" "," "List of integers");

  OZ_Term var_list = oz_deref(OZ_in(0));
  OZ_Term id_list = OZ_nil();

  VarListExpect vle;

  EXPECT_BLOCK(vle, 0, expectListVar, "List of variables");

  int length = OZ_length(var_list);

  OZ_Term vars[length];

  for (int i = 0; !OZ_isNil(var_list); var_list = OZ_tail(var_list), i += 1) {
    vars[i] = OZ_head(var_list);
  }

  int * ids = OZ_findEqualVars(length, vars);

  for (int i = length; i--; ) {
    id_list = OZ_cons(ids[i] < 0 ? atom_nonevar : OZ_int(ids[i] + 1),
		      id_list);
  }

  DEBUGPRINT(("BIIdentifyParameter out\n"));

  OZ_RETURN(id_list);
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIIsActivePropagator, 1, 1)
{
  DEBUGPRINT(("BIIsActivePropagator in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (!oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = tagged2Extension(v1);
  if (PropagatorReference::getId() != se1->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_RETURN(((PropagatorReference *) se1)->isActive() 
	    ? oz_true() : oz_false());
} OZ_BI_end

OZ_BI_define(BIDeactivatePropagator, 1, 0)
{
  DEBUGPRINT(("BIDeactivatePropagator in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  ((PropagatorReference *) se1)->deactivate();

  DEBUGPRINT(("BIDeactivatePropagator out\n"));

  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIActivatePropagator, 1, 0)
{
  DEBUGPRINT(("BIActivatePropagator in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  ((PropagatorReference *) se1)->activate();

  DEBUGPRINT(("BIActivatePropagator out\n"));

  return PROCEED;
} OZ_BI_end


//-----------------------------------------------------------------------------

OZ_BI_define(BIIsPropagator, 1, 1)
{
  DEBUGPRINT(("BIIsPropagator in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (!oz_isExtension(v1)) {
    OZ_RETURN(oz_false());
  }

  OZ_Extension * se1 = tagged2Extension(v1);
  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
  }

  OZ_RETURN(oz_true());
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIIsDiscardedPropagator, 1, 1)
{
  DEBUGPRINT(("BIIsDiscardedPropagator in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }


  DEBUGPRINT(("BIIsDiscardedPropagator out\n"));

  OZ_RETURN(((PropagatorReference *) se1)->isDiscarded()
	    ? oz_true() : oz_false());
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIDiscardPropagator, 1, 0)
{
  DEBUGPRINT(("BIDiscardPropagator in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  ((PropagatorReference *) se1)->discard();

  DEBUGPRINT(("BIDiscardPropagator out\n"));

  return PROCEED;
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIPropagatorEq, 2, 1)
{
  DEBUGPRINT(("BIPropagatorEq in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (!oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = tagged2Extension(v1);
  if (PropagatorReference::getId() != se1->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Term v2 = OZ_in(1);

  if (!oz_isExtension(v2)) {
    return OZ_raiseErrorC(EXCEPTION, 1, EXPECT_PROPGATORREF, v2);
  }

  OZ_Extension * se2 = tagged2Extension(v2);
  if (PropagatorReference::getId() != se2->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 1, EXPECT_PROPGATORREF, v2);
  }

  DEBUGPRINT(("BIPropagatorEq out\n"));

  OZ_RETURN(* ((PropagatorReference *) se1) == *((PropagatorReference *) se2)
	    ? oz_true() : oz_false());
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIReflectPropagator, 1, 1)
{
  DEBUGPRINT(("BIReflectPropagator in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  if (((PropagatorReference*) se1)->isDiscarded()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORDISCARDED, v1);
  }

  OZ_Term r =
    reflect_propagator(((PropagatorReference*) se1)->getPropagator());

  DEBUGPRINT(("BIReflectPropagator out\n"));

  OZ_RETURN(r);
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIReflectPropagatorName, 1, 1)
{
  DEBUGPRINT(("BIReflectPropagatorName in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
  }

  if (((PropagatorReference*) se1)->isDiscarded()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORDISCARDED, v1);
  }

  OZ_Term r = prop_name(((PropagatorReference*) se1)
			->getPropagator()->getPropagator()
			->getProfile()->getPropagatorName());

  DEBUGPRINT(("BIReflectPropagatorName out\n"));

  OZ_RETURN(r);
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIIsPropagatorFailed, 1, 1)
{
  DEBUGPRINT(("BIIsPropagatorFailed in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
  }

  if (((PropagatorReference*) se1)->isDiscarded()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORDISCARDED, v1);
  }

  Propagator * p = ((PropagatorReference*) se1)->getPropagator();

  DEBUGPRINT(("BIIsPropagatorFailed out\n"));

  OZ_RETURN(p->isFailed() ? oz_true() : oz_false());
} OZ_BI_end


//-----------------------------------------------------------------------------

OZ_BI_define(BIReflectPropagatorCoordinates, 1, 1)
{
  DEBUGPRINT(("BIReflectPropagatorCoordinates in\n"));

  OZ_Term v1 = oz_deref(OZ_in(0));

  if (! oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
  }

  if (((PropagatorReference*) se1)->isDiscarded()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORDISCARDED, v1);
  }

  OZ_Term r = oz_propGetName(((PropagatorReference*) se1)->getPropagator());

  DEBUGPRINT(("BIReflectPropagatorCoordinates out\n"));

  OZ_RETURN(r);
} OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIReflectVariable, 1, 1)
{
  OZ_RETURN(reflect_variable(OZ_in(0)));
}
OZ_BI_end

//-----------------------------------------------------------------------------

OZ_BI_define(BIReflectSpace, 1, 1)
{
  OZ_RETURN(reflect_space(OZ_in(0)));
}
OZ_BI_end
