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

#include "intsets.hh"
#include "monitor.hh"
#include "reified.hh"
#include "standard.hh"
#include "std_n.hh"
#include "telling.hh"
#include "testing.hh"

void fsp_init(void)
{
#ifdef OZ_DEBUG
  oz_fsetdebugprint("*** DEBUG-FSETLIB ***");
#elif defined(OZ_PROFILE)
  oz_fsetdebugprint("*** PROFILE-FSETLIB ***");
#endif

  FSetsMinPropagator::profile         = "fsp_min";
  FSetsMaxPropagator::profile         = "fsp_max";
  FSetsConvexPropagator::profile      = "fsp_convex";
  FSetMatchPropagator::profile        = "fsp_match";
  FSetMinNPropagator::profile         = "fsp_minN";
  FSetMaxNPropagator::profile         = "fsp_maxN";
  FSetSeqPropagator::profile          = "fsp_seq";
  MonitorInPropagator::profile        = "fsp_monitorIn";
  IsInRPropagator::profile            = "fsp_isInR";
  BoundsPropagator::profile           = "fsp_bounds";
  BoundsNPropagator::profile          = "fsp_boundsN";
  PartitionReifiedPropagator::profile = "fsp_partitionReified";
  EqualRPropagator::profile           = "fsp_equalR";
  IncludeRPropagator::profile         = "fsp_includeR";
  FSetIntersectionPropagator::profile = "fsp_intersection";
  FSetUnionPropagator::profile        = "fsp_union";
  FSetSubsumePropagator::profile      = "fsp_subsume";
  FSetDisjointPropagator::profile     = "fsp_disjoint";
  FSetDistinctPropagator::profile     = "fsp_distinct";
  FSetDiffPropagator::profile         = "fsp_diff";
  FSetUnionNPropagator::profile       = "fsp_unionN";
  FSetDisjointNPropagator::profile    = "fsp_disjointN";
  FSetPartitionPropagator::profile    = "fsp_partition";
  IncludePropagator::profile          = "fsp_include";
  ExcludePropagator::profile          = "fsp_exclude";
  FSetCardPropagator::profile         = "fsp_card";
  IsInPropagator::profile             = "fsp_isIn";
}
