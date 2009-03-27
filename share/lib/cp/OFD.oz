%%%
%%% Authors:
%%%     Gustavo A. Gomez Farhat <gafarhat@univalle.edu.co>
%%%
%%% Copyright:
%%%     Gustavo A. Gomez Farhat, 2009
%%%
%%% Last change:
%%%   $Date: $ by $Author: $
%%%   $Revision: $
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

import
   FD

export
   %% Telling Domains
   int:            FdInt
   bool:           FdBool
   dom:            FdDom
   decl:           FdDecl
   list:           FdList
   tuple:          FdTuple
   record:         FdRecord
   
   %% Reflection
   reflect:        FdReflect
   
   %% Watching Domains
   watch:          FdWatch
   
   %% Generic Propagators
   sum:            FdpSum
   sumC:           FdpSumC
   sumCN:          FdpSumCN
   sumAC:          FdpSumAC
   sumACN:         GenSumACN
   sumD:           FdpDSum
   sumCD:          FdpDSumC
   
   %% Symbolic Propagators
   distinct:       FdpDistinct
   distinct2:      FdpDistinct2
   distinctB:      FdpDistinctB
   distinctD:      FdpDistinctD
   distinctOffset: FdpDistinctOffset
   atMost:         FdpAtMost
   atLeast:        FdpAtLeast
   exactly:        FdpExactly
   element:        FdpElement
   
   %% 0/1 Propagators
   conj:           FdpConj
   disj:           FdpDisj
   nega:           FdpNega
   exor:           FdpExor
   impl:           FdpImpl
   equi:           FdpEqui
   
   %% Reified Propagators
   reified:        FdReified
   
   %% Miscellaneous Propagators
   plus:           FdpPlus
   minus:          FdpMinus 
   times:          FdpTimes 
   plusD:          FdpPlusD
   minusD:         FdpMinusD 
   timesD:         FdpTimesD
   power:          FdpPower 
   divI:           FdpDivI
   divD:           FdpDivD
   modI:           FdpModI
   modD:           FdpModD
   max:            FdpMaximum
   min:            FdpMinimum
   distance:       FdpDistance
   tasksOverlap:   FdpTasksOverlap
   less:           FdLess
   greater:        FdGreater
   lesseq:         FdLesseq
   greatereq:      FdGreatereq
   disjointC:      FdDisjointC
   disjoint:       FdpDisjoint
   
   %% Distribution
   assign:         FdAssign
   distribute:     FdDistribute
   choose:         FdChoose
   
   %% Miscellaneous
   inf:            FdInf
   sup:            FdSup
   is:             FdIs

define

   %% Telling Domains
   FdInt             = FD.int
   FdBool            = FD.bool
   FdDom             = FD.dom
   FdDecl            = FD.decl
   FdList            = FD.list
   FdTuple           = FD.tuple
   FdRecord          = FD.record
   
   %% Reflection
   FdReflect         = FD.reflect

   %% Watching Domains
   FdWatch           = FD.watch

   %% Generic Propagators
   FdpSum            = FD.sum
   FdpSumC           = FD.sumC
   FdpSumCN          = FD.sumCN
   FdpSumAC          = FD.sumAC
   GenSumACN         = FD.sumACN
   FdpDSum           = FD.sumD
   FdpDSumC          = FD.sumCD
   
   %% Symbolic Propagators
   FdpDistinct       = FD.distinct
   FdpDistinct2      = FD.distinct2
   FdpDistinctB      = FD.distinctB
   FdpDistinctD      = FD.distinctD
   FdpDistinctOffset = FD.distinctOffset
   FdpAtMost         = FD.atMost
   FdpAtLeast        = FD.atLeast
   FdpExactly        = FD.exactly
   FdpElement        = FD.element
   
   %% 0/1 Propagators   
   FdpConj           = FD.conj
   FdpDisj           = FD.disj
   FdpNega           = FD.nega
   FdpExor           = FD.exor
   FdpImpl           = FD.impl
   FdpEqui           = FD.equi
   
   %% Reified Propagators
   FdReified         = FD.reified
   
   %% Miscellaneous Propagators
   FdpPlus           = FD.plus
   FdpMinus          = FD.minus
   FdpTimes          = FD.times
   FdpPlusD          = FD.plusD
   FdpMinusD         = FD.minusD
   FdpTimesD         = FD.timesD
   FdpPower          = FD.power
   FdpDivI           = FD.divI
   FdpDivD           = FD.divD
   FdpModI           = FD.modI
   FdpModD           = FD.modD
   FdpMaximum        = FD.max
   FdpMinimum        = FD.min
   FdpDistance       = FD.distance
   FdpTasksOverlap   = FD.tasksOverlap
   FdLess            = FD.less
   FdGreater         = FD.greater
   FdLesseq          = FD.lesseq
   FdGreatereq       = FD.greatereq
   FdDisjointC       = FD.disjointC
   FdpDisjoint       = FD.disjoint
   
   %% Distribution
   FdAssign          = FD.assign
   FdDistribute      = FD.distribute
   FdChoose          = FD.choose
   
   %% Miscellaneous
   FdInf             = FD.inf
   FdSup             = FD.sup
   FdIs              = FD.is

end
