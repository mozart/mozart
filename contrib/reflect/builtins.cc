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
OZ_atom("Expecting Situated Extension (Propagator Reference)")

#define EXCEPTION \
"visualize_constraints_error"

OZ_BI_define(BIPropagatorEq, 2, 1)
{
  DEBUGPRINT(("BIPropagatorEq in\n"));
  
  OZ_Term v1 = oz_deref(OZ_in(0));

  if (!oz_isExtension(v1)) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }

  OZ_Extension * se1 = oz_tagged2Extension(v1);
  if (PropagatorReference::getId() != se1->getIdV()) {
    return OZ_raiseErrorC(EXCEPTION, 0, EXPECT_PROPGATORREF, v1);
  }
      
  OZ_Term v2 = OZ_in(1);

  if (!oz_isExtension(v2)) {
    return OZ_raiseErrorC(EXCEPTION, 1, EXPECT_PROPGATORREF, v2);
  }
  
  OZ_Extension * se2 = oz_tagged2Extension(v2);
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
  
  OZ_Extension * se1 = oz_tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
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
  
  OZ_Extension * se1 = oz_tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
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
  
  OZ_Extension * se1 = oz_tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
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
  
  OZ_Extension * se1 = oz_tagged2Extension(v1);

  if (PropagatorReference::getId() != se1->getIdV()) {
    OZ_RETURN(oz_false());
  }

  OZ_Term r = oz_propGetName(((PropagatorReference*) se1)
			     ->getPropagator());

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
