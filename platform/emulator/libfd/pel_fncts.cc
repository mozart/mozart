/*
 *  Authors:
 *    Joerg Wuertz (wuertz@dfki.de)
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

#include "rel.hh"
#include "pel_fncts.hh"

template class _OZ_ParamIterator<int>;
template class _PropagatorController_V_V<int, PEL_FDIntVar, 1, 0, 2>;
template class _PropagatorController_V_V_V<unsigned int, OZ_FDIntVar, 1, 0, 3>;

template PEL_Filter<PEL_FDIntVar> & filter_lessEqOffset<PEL_Filter<PEL_FDIntVar>, PEL_FDIntVar>(PEL_Filter<PEL_FDIntVar> &, PEL_FDIntVar &, PEL_FDIntVar &, int);
