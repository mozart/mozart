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
 *     http://www.mozart-oz.org/
 *
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __PROP_FNCTS_HH__
#define __PROP_FNCTS_HH__

#include "prop_engine.hh"

//-----------------------------------------------------------------------------

// X + C <= Y
pf_return_t lessEqOff(int *, PEL_SuspVar * []);

// X + C > Y
pf_return_t greaterOff(int *, PEL_SuspVar * []);

#endif
