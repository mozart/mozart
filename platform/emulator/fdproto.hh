/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

//*****************************************************************************
// Finite Domains

// fdprof.cc
OZ_C_proc_proto(BIfdReset)
OZ_C_proc_proto(BIfdDiscard)
OZ_C_proc_proto(BIfdGetNext)
OZ_C_proc_proto(BIfdPrint)
OZ_C_proc_proto(BIfdTotalAverage)

// fdcore.cc
OZ_C_proc_proto(BIisFdVar)
OZ_C_proc_proto(BIisFdVarB)
OZ_C_proc_proto(BIfdIs)
OZ_C_proc_proto(BIgetFDLimits)
OZ_C_proc_proto(BIfdMin)
OZ_C_proc_proto(BIfdMid)
OZ_C_proc_proto(BIfdMax)
OZ_C_proc_proto(BIfdGetAsList)
OZ_C_proc_proto(BIfdGetCardinality)
OZ_C_proc_proto(BIfdTellConstraint)
OZ_C_proc_proto(BIfdNextSmaller)
OZ_C_proc_proto(BIfdNextLarger)
OZ_C_proc_proto(BIfdWatchSize)
OZ_C_proc_proto(BIfdWatchMin)
OZ_C_proc_proto(BIfdWatchMax)

// fdcd.cc
OZ_C_proc_proto(BIfdConstrDisjSetUp)
OZ_C_proc_proto(BIfdConstrDisj)

OZ_C_proc_proto(BIfdTellConstraintCD)

// fddist.cc
OZ_C_proc_proto(BIfdDistribute)
OZ_C_proc_proto(BIfdGetCandidates)
OZ_C_proc_proto(BIfdDistributeTaskIntervals)
OZ_C_proc_proto(BIfdDistributeTaskIntervalsOpt)
OZ_C_proc_proto(BIfdDistributeMinPairs)

#ifndef FOREIGNFDPROPS
OZ_C_proc_proto(fdp_init)
OZ_C_proc_proto(fdp_sum)
OZ_C_proc_proto(fdp_sumC)
OZ_C_proc_proto(fdp_sumCN)
OZ_C_proc_proto(fdp_sumR)
OZ_C_proc_proto(fdp_sumCR)
OZ_C_proc_proto(fdp_sumCNR)
OZ_C_proc_proto(fdp_sumCD)
OZ_C_proc_proto(fdp_sumCCD)
OZ_C_proc_proto(fdp_sumCNCD)
OZ_C_proc_proto(fdp_plus_rel)
OZ_C_proc_proto(fdp_plus)
OZ_C_proc_proto(fdp_minus)
OZ_C_proc_proto(fdp_times)
OZ_C_proc_proto(fdp_times_rel)
OZ_C_proc_proto(fdp_power)
OZ_C_proc_proto(fdp_divD)
OZ_C_proc_proto(fdp_divI)
OZ_C_proc_proto(fdp_modD)
OZ_C_proc_proto(fdp_modI)
OZ_C_proc_proto(fdp_conj)
OZ_C_proc_proto(fdp_disj)
OZ_C_proc_proto(fdp_exor)
OZ_C_proc_proto(fdp_impl)
OZ_C_proc_proto(fdp_equi)
OZ_C_proc_proto(fdp_nega)
OZ_C_proc_proto(fdp_intR)
OZ_C_proc_proto(fdp_card)
OZ_C_proc_proto(fdp_exactly)
OZ_C_proc_proto(fdp_atLeast)
OZ_C_proc_proto(fdp_atMost)
OZ_C_proc_proto(fdp_element)
OZ_C_proc_proto(fdp_notEqOff)
OZ_C_proc_proto(fdp_lessEqOff)
OZ_C_proc_proto(fdp_minimum)
OZ_C_proc_proto(fdp_maximum)
OZ_C_proc_proto(fdp_inter)
OZ_C_proc_proto(fdp_union)
OZ_C_proc_proto(fdp_distinct)
OZ_C_proc_proto(fdp_distinctStream)
OZ_C_proc_proto(fdp_distinctOffset)
OZ_C_proc_proto(fdp_disjoint)
OZ_C_proc_proto(sched_disjoint_card)
OZ_C_proc_proto(fdp_disjointC)
OZ_C_proc_proto(fdp_distance)
OZ_C_proc_proto(sched_cpIterate)
OZ_C_proc_proto(sched_cpIterateCap)
OZ_C_proc_proto(sched_cpIterateCapUp)
OZ_C_proc_proto(sched_taskIntervals)
OZ_C_proc_proto(sched_disjunctive)
OZ_C_proc_proto(sched_disjunctiveStream)
OZ_C_proc_proto(sched_taskIntervalsProof)
OZ_C_proc_proto(sched_firstsLasts)

OZ_C_proc_proto(fdp_dsum)
OZ_C_proc_proto(fdp_dsumC)
OZ_C_proc_proto(fdp_sumAC)

// dummies
OZ_C_proc_proto(fdp_twice)
OZ_C_proc_proto(fdp_square)
OZ_C_proc_proto(fdp_subset)

OZ_C_proc_proto(fdtest_counter)
OZ_C_proc_proto(fdtest_firstFail)
OZ_C_proc_proto(fdtest_spawnLess)
OZ_C_proc_proto(fdtest_plus)
OZ_C_proc_proto(fdtest_sumac)

//*****************************************************************************
//  Finite Sets

OZ_C_proc_proto(fsp_init)
OZ_C_proc_proto(fsp_isIn)
OZ_C_proc_proto(fsp_include)
OZ_C_proc_proto(fsp_exclude)
OZ_C_proc_proto(fsp_card)
OZ_C_proc_proto(fsp_intersection)
OZ_C_proc_proto(fsp_union)
OZ_C_proc_proto(fsp_subsume)
OZ_C_proc_proto(fsp_disjoint)
OZ_C_proc_proto(fsp_distinct)
OZ_C_proc_proto(fsp_monitorIn)
OZ_C_proc_proto(fsp_min)
OZ_C_proc_proto(fsp_max)
OZ_C_proc_proto(fsp_convex)
OZ_C_proc_proto(fsp_diff)
OZ_C_proc_proto(fsp_includeR)
#endif
