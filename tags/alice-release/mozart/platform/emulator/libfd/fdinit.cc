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

#include "base.hh"
#include "streamProps.hh"
#include "arith.hh"
#include "bool.hh"
#include "card.hh"
#include "complalldist.hh"
#include "boundsalldist.hh"
#include "count.hh"
#include "diffn.hh"
#include "disjoint.hh"
#include "distance.hh"
#include "rel.hh"
#include "sum.hh"
#include "sumabs.hh"
#include "sumd.hh"
#include "distribute.hh"
#include "taskoverlap.hh"

void fdp_init(void) 
{
  fd_dist_init();

#ifdef OZ_DEBUG
  oz_debugprint("*** DEBUG-FDLIB ***");
#elif defined(OZ_PROFILE)
  oz_debugprint("*** PROFILE-FDLIB ***");
#endif

  TwicePropagator::profile =             "fdp_twice";
  SquarePropagator::profile =            "fdp_square";
  PlusPropagator::profile =              "fdp_plus";
  TimesPropagator::profile =             "fdp_times";
  TwiceDPropagator::profile =            "fdp_twiceD";
  SquareDPropagator::profile =           "fdp_squareD";
  PlusDPropagator::profile =             "fdp_plusD";
  TimesDPropagator::profile =            "fdp_timesD";
  DivPropagator::profile =               "fdp_divD";
  DivIPropagator::profile =              "fdp_divI";
  ModPropagator::profile =               "fdp_modD";
  ModIPropagator::profile =              "fdp_modI";
  PowerPropagator::profile =             "fdp_power";
  ConjunctionPropagator::profile =       "fdp_conj";
  DisjunctionPropagator::profile =       "fdp_disj";
  XDisjunctionPropagator::profile =      "fdp_exor";
  ImplicationPropagator::profile =       "fdp_impl";
  EquivalencePropagator::profile =       "fdp_equi";
  NegationPropagator::profile =          "fdp_nega";
  LinEqBPropagator::profile =            "fdp_sumCR";
  LinNotEqBPropagator::profile =         "fdp_sumCR";
  LinLessEqBPropagator::profile =        "fdp_sumCR";
  InBPropagator::profile =               "fdp_intR";
  CardBPropagator::profile =             "fdp_card";
  CompleteAllDistProp::profile =         "fdp_distinctD";
  BoundsDistinctPropagator::profile =    "fdp_distinctB";
  ExactlyPropagator::profile =           "fdp_exactly";
  AtLeastPropagator::profile =           "fdp_atLeast";
  AtMostPropagator::profile =            "fdp_atMost";
  ElementPropagator::profile =           "fdp_element"; 
  DiffnPropagator::profile =             "fdp_distinct2";
  SchedCDPropagator::profile =           "fdp_disjoint";
  SchedCDBPropagator::profile =          "fdp_disjointC";
  TasksOverlapPropagator::profile =      "fdp_tasksOverlap";
  DistancePropagatorLeq::profile =       "fdp_distance";
  DistancePropagatorGeq::profile =       "fdp_distance";
  DistancePropagatorEq::profile =        "fdp_distance";
  DistancePropagatorNeq::profile =       "fdp_distance";
  NotEqOffPropagator::profile =          "fdp_notEqOff";
  LessEqOffPropagator::profile =         "fdp_lessEqOff";
  MinimumPropagator::profile =           "fdp_minimum";
  MaximumPropagator::profile =           "fdp_maximum";
  IntersectionPropagator::profile =      "fdp_inter";
  UnionPropagator::profile =             "fdp_union";
  DistinctPropagator::profile =          "fdp_distinct";
  DistinctOffsetPropagator::profile =    "fdp_distinctOffset";
  SubSetPropagator::profile  =           "fdp_subset";
  DistinctPropagatorStream::profile =    "fdp_distinctStream";
  LinEqPropagator::profile =             "fdp_sumC";
  LinNotEqPropagator::profile =          "fdp_sumC";
  LinLessEqPropagator::profile =         "fdp_sumC";
  NonLinEqPropagatorP::profile =         "fdp_sumCN";
  NonLinLessEqPropagatorP::profile =     "fdp_sumCN";
  LinEqAbsPropagator::profile =          "fdp_sumAC";
  LinLessEqAbsPropagator::profile =      "fdp_sumAC";
  LinGreaterEqAbsPropagator::profile =   "fdp_sumAC";
  LinNotEqAbsPropagator::profile =       "fdp_sumAC";
  isumEqProp::profile =                  "fdp_dsum";
  isumNEqProp::profile =                 "fdp_dsum";
  isumcEqProp::profile =                 "fdp_dsumC";
  isumcNEqProp::profile =                "fdp_dsumC";

  CompleteAllDistProp::init_memory_management = 1;
}

