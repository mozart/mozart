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
 *     $MOZARTURL$
 * 
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and rfedistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

% This is a rudimentary test suite for the real interval constraints
% which serve as an example for the functionality of the CPI to
% implement constraint systems from scratch.

% On success, the Browser displays:
% [1 1 1 1 1 1 1 1 1 1 1 1 1]
% [[1.0 3.0]]
% [[~2.0] [2.0]]		

% You are welcome to find new test cases and to add them to this file.
% -- Tobias (tmueller@ps.uni-sb.de)

declare [RI LP] = {Module.link ['x-oz://contrib/RI' 'x-oz://contrib/LP']}
{Wait RI}
{Wait LP}

%{LP.config put config(solver: cplex_primopt)}

{Show RI#LP}

{Browse
 [
  {LP.config get}

  {fun {$} X = {RI.var.bounds 1.4 1.5} Y = {RI.var.bounds 1.5 1.7} R
   in
      R = thread cond X = Y then 1 else 0 end end
      X = Y
      R
   end}

  {fun {$} X= {RI.var.bounds 1.5 1.7} R
   in
     R = thread cond X = 1.6 then 1 else 0 end end
     X = 1.6
     R
  end}

 {fun {$} X = {RI.var.bounds 2.0 4.5} Y = {RI.var.bounds 1.5 3.0} R in
     R = thread cond {RI.lessEq X Y} then 1 else 0 end end
     {RI.lessEq X 2.5}
     {RI.lessEq 2.5 Y}
     R
  end}

 {fun {$} X = {RI.var.bounds 1.5 3.0}  Y = {RI.var.bounds 2.0 4.5} R in
     R = thread cond {RI.lessEq X Y} then 0 else 1 end end
     {RI.greater X 2.5}
     {RI.greater 2.5 Y}
     R
  end}
 
 {fun {$} X = {RI.var.bounds 2.0 4.5} Y = {RI.var.bounds 1.5 3.0} R in
     R = thread cond {RI.greater X Y} then 0 else 1 end end
     {RI.lessEq X 2.4}
     {RI.lessEq 2.5 Y}
     R
  end}

 {fun {$} X = {RI.var.bounds 1.5 3.0} Y = {RI.var.bounds 2.0 4.5} R in
     R = thread cond {RI.greater X Y} then 1 else 0 end end
     {RI.greater X 2.5}
     {RI.greater 2.5 Y}
     R
  end}

 {fun {$} F1 = {RI.var.bounds ~1.0 2.5} F2 = {RI.var.bounds 1.5 3.0} R in
     R = thread cond F1 = 1.5 F2 = 1.5 then 1 else 0 end end
     F1 = F2
     {RI.var.bounds 1.5 1.51 F1}
     F1 = 1.5
     R
  end}

 {fun {$} X = {RI.var.bounds 2.0 4.5} Y = {RI.var.bounds 1.5 3.0} R in
     R = thread cond {RI.lessEq X Y} then 1 else 0 end end
     {RI.lessEq X 2.5}
     {RI.lessEq 2.5 Y}
     R
  end}
 
 {fun {$} F1 = {RI.var.bounds ~1.0 2.5} F2 = {RI.var.bounds 1.5 3.0} R in     
     R = thread cond F1 = F2 then 1 else 0 end end
     F1=F2
     R 
  end}
 
 {fun {$} A = {RI.var.bounds 1.0 2.0} B = {RI.var.bounds 2.0 3.0} C = {RI.var.bounds 3.0 5.0}
     R in
     {RI.plus A B C}
     R = thread cond {RI.plus A B C} then 1 else 0 end end
     C=5.0
     R
  end}

 {fun {$} A = {RI.var.bounds ~1.0 2.0} B = {RI.var.bounds 2.0 3.0} C = {RI.var.bounds 4.0 7.0}
     R in
     {RI.times A B C}
     R = thread cond {RI.times A B C} then 1 else 0 end end
     C = 6.0
     R
  end}

 {fun {$} A = {RI.var.bounds ~1.0 2.0} B = {RI.var.bounds 2.0 3.0} C = {RI.var.bounds 4.0 7.0} R in
     {RI.times A B C}
     R = thread cond {RI.times A B C} then 1 else 0 end end
     A = 2.0
     B = 3.0
     R
  end}
 
 {fun {$} F = {RI.var.bounds 1.0 2.0}
     L = {RI.getLowerBound F}
     U = {RI.getUpperBound F}
  in
     cond L = 1.0 U = 2.0 then 1 else 0 end
  end}

 {fun{$}
     R F = {RI.var.bounds ~10.5 10.5} D = {FD.decl}
  in
     {RI.intBounds F D}

     R = thread cond D = 4 F = 4.0 then 1 else 0 end end
     
     {RI.lessEq F 5.5}
     {RI.greater F 1.0}
     D <: 5
     D >: 3
     
     R
  end}


 {fun {$} R F in 
     R = thread cond F = 2.0 then 1 else 0 end end
     {RI.var.decl F} {RI.var.bounds 1.0 2.0 F} {RI.var.bounds 2.0 3.0 F}
     R
  end}

 {fun {$} A B R in
     thread {RI.lessEq A B} end
     R = thread cond {RI.var.decl B} then 1 else 0 end end
     {RI.var.decl A}
     R
  end}

 {fun {$} A B C R in
     {RI.var.decl B}
     thread {RI.plus A B C} end
     R = thread cond {RI.var.decl C} then 1 else 0 end end
     {RI.var.decl A}
     R
  end}

 {fun {$} A B R in
     thread {RI.intBounds A B} end
     R = thread cond {FD.decl B} then 1 else 0 end end
     {RI.var.decl A}
     R
  end}

 {fun {$} A B R in
     thread {RI.intBounds A B} end
     R = thread cond {RI.var.decl A} then 1 else 0 end end
     {FD.decl B}
     R
  end}


 ]}

{Browse {SearchOne proc {$ Sol} [A B] = Sol in
                      {ForAll Sol proc {$ V} {RI.var.bounds 1.0 3.0 V} end}
		      {RI.times A B 3.0}
		      {ForAll Sol RI.distribute} 
		   end}}


{Browse {SearchAll proc {$ Sol} [X] = Sol in
			 {ForAll Sol proc {$ V} {RI.var.bounds ~10.0 10.0 V} end}
			 {RI.times X X 4.0}
			 {ForAll Sol RI.distribute} 

		   end}}

local
   fun {SelectVar VsPair}
      case VsPair
      of nil#nil then unit#unit 
      [] (VH|VT)#(RVH|RVT) then
      % check for integrality
	 if RVH == {Round RVH}
	 then {SelectVar VT#RVT}
	 else VH#RVH
	 end
      else
	 {Browse VsPair} unit
      end
   end   
   fun {DuplicateRIs Vs}
      {Map Vs fun {$ V} {RI.var.bounds
			 {RI.getLowerBound V}
			 {RI.getUpperBound V}}
	      end}	 
   end
   proc {DistributeKnapSackLP Vs ObjFn Constraints MaxProfit}
      {Space.waitStable}
{Show gaga}
      local
	 DupVs = {DuplicateRIs Vs}
	 DupMaxProfit V DupV
      in
	 DupMaxProfit = {RI.var.bounds
			 {RI.getLowerBound MaxProfit}
			 {RI.getUpperBound MaxProfit}}
{Show 1}
	 
	 {LP.solve DupVs ObjFn Constraints DupMaxProfit optimal}
{Show 2}
	 V#DupV = {SelectVar Vs#DupVs}
	 
	 if {IsDet V} then
{Show 3}
	    DupMaxProfit = MaxProfit
	    DupVs        = Vs
	 else
{Show 4}
	    dis {RI.lessEq {Ceil DupV} V} 
	    then
{Show gaga1}
	       {DistributeKnapSackLP Vs ObjFn Constraints MaxProfit}
	    [] {RI.lessEq V {Floor DupV}} 
	    then
{Show gaga2}
	       {DistributeKnapSackLP Vs ObjFn Constraints MaxProfit}
	    end
	 end     
      end
   end
   fun {KnapsackFDLP Problem}
      NumProducts = {Length Problem.profit}
      Resources   = Problem.resources
   in
      proc {$ Sol}
	 sol(maxprofit: FDMaxProfit = {FD.decl}
	     products:  FDProducts  = {FD.list NumProducts 0#FD.sup}
	     profit:    Problem.profit) = Sol
	 
	 ObjFn Constraints
	 MaxProfit = {RI.var.decl}
	 Products  = {MakeList NumProducts}
      in
	 FDMaxProfit = {FD.sumC Problem.profit FDProducts '=:'}
	 
	 {ForAll {Arity Resources}
	  proc {$ ResourceName}
	     Resource = Resources.ResourceName
	  in
	     {FD.sumC Resource.npp FDProducts '=<:' Resource.ta}
	  end}
	 {ForAll Products proc {$ V} {RI.var.bounds 0.0 RI.sup V} end}
	 ObjFn = objfn(row: {Map Problem.profit fun {$ I} {IntToFloat I} end}
		       opt: max)
	 Constraints = 
	 {Map {Arity Resources}
	  fun {$ ResourceName}
	  Resource = Resources.ResourceName
	  in
	     constr(row: {Map Resource.npp fun {$ I} {IntToFloat I} end}
		    type: '=<'
		    rhs: {IntToFloat Resource.ta})
	  end}
	 {RI.intBounds MaxProfit FDMaxProfit}
	 {Map Products proc {$ R D} {RI.intBounds R D} end FDProducts}
	 {DistributeKnapSackLP Products ObjFn Constraints MaxProfit}
      end
   end
   Problem =
   problem(
      resources: resource(
		    man:       r(ta: 14 npp: [1 1 1 1 1 1 1 1 1 1 1 1 1])
		    material1: r(ta: 17 npp: [0 4 5 0 0 0 0 4 5 0 0 0 0])
		    material2: r(ta: 20 npp: [4 0 3 0 0 0 3 0 4 0 0 0 0])
		    machine1:  r(ta: 34 npp: [7 0 0 6 0 0 7 0 0 6 0 0 0])
		    machine2:  r(ta: 26 npp: [0 0 0 4 5 0 0 0 0 5 4 0 0])
		    machine3:  r(ta: 16 npp: [0 0 0 0 4 3 0 0 0 0 4 2 1])
		    machine4:  r(ta: 16 npp: [0 3 0 0 0 5 0 3 0 0 0 3 3]))
      profit: [5 7 5 11 8 10 6 8 3 12 9 8 4])
in
skip
   {Browse {SearchBest {KnapsackFDLP Problem}
	    proc {$ O N} O.maxprofit <: N.maxprofit end}}
%   {ExploreBest {KnapsackFDLP Problem}
%    proc {$ O N} O.maxprofit <: N.maxprofit end}
end

