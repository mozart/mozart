%%%
%%% Authors:
%%%   Tobias Müller <tmueller@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Tobias Müller, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
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

declare

local
   Lit2Int = {NewName}
   Int2Lit = {NewName}
in
   fun {SetOfLiterals Lits}
      sol(Lit2Int:
	     {NewChunk
	      {List.toRecord l2i {List.mapInd Lits fun {$ I L} L#I end}}}
	  Int2Lit:
	     {NewChunk
	      {List.toRecord i2l {List.mapInd Lits fun {$ I L} I#L end}}})
   end
   
   fun {Lits2Ints SetOfLiterals Literals}
      {Map Literals fun {$ Lit} SetOfLiterals.Lit2Int.Lit end}
   end
   
   fun {Ints2Lits SetOfLiterals Ints}
      {Map Ints fun {$ Int} SetOfLiterals.Int2Lit.Int end}
   end
end

fun {CrewProb FlightData Crew}
   CabinStaff      = {Append Crew.stewards Crew.stewardesses}
   CrewSet         = {SetOfLiterals CabinStaff}
   Stewards        = {FS.value.make {Lits2Ints CrewSet Crew.stewards}}
   Stewardesses    = {FS.value.make {Lits2Ints CrewSet Crew.stewardesses}}
   FrenchSpeaking  = {FS.value.make {Lits2Ints CrewSet Crew.frenchspeaking}}
   GermanSpeaking  = {FS.value.make {Lits2Ints CrewSet Crew.germanspeaking}}
   SpanishSpeaking = {FS.value.make {Lits2Ints CrewSet Crew.spanishspeaking}}

   proc {TeamConstraint Team Flight}
      flight(no:_ crew:N stewards:NStew stewardesses:NHost
	     frenchspeaking:NFrench germanspeaking:NGerman
	     spanishspeaking:NSpanish) = Flight
   in
      {FS.card Team  N}
      {FS.card {FS.intersect Team Stewards}}        >=: NStew
      {FS.card {FS.intersect Team Stewardesses}}    >=: NHost
      {FS.card {FS.intersect Team FrenchSpeaking}}  >=: NFrench
      {FS.card {FS.intersect Team GermanSpeaking}}  >=: NGerman
      {FS.card {FS.intersect Team SpanishSpeaking}} >=: NSpanish
   end
	      
   proc {SequencedDisjoint L}
      case L of A|B|C|T then
	 {FS.disjoint A B}
	 {FS.disjoint A C}
	 {SequencedDisjoint B|C|T}
      elseof A|B|nil then
	 {FS.disjoint A B}
      end
   end
in
   proc {$ Sol}
      Flights = {FS.var.list.upperBound
		 {Length FlightData} {Lits2Ints CrewSet CabinStaff}}
   in      
      {Map FlightData proc {$ D F} {TeamConstraint F D} end Flights}
		 
      {SequencedDisjoint Flights}

      {FS.distribute naive Flights}

      Sol = {Map Flights fun {$ F} {Ints2Lits CrewSet {FS.monitorIn F}} end}
   end
end
