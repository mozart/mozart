%%%
%%% Authors:
%%%     Andres Felipe Barco <anfelbar@univalle.edu.co>
%%%
%%% Copyright:
%%%     Andres Barco, 2008
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

%%% Adapted from "Problem Solving With Finite Set Constraints in Oz. A Tutorial"
%%% by Tobias Muller, 1998

declare 
local 
   Lit2Int = {NewName}
   Int2Lit = {NewName}
in 
   fun {SetOfLiterals Lits}
      sol(Lit2Int:
             {NewChunk
              {List.toRecord l2i
               {List.mapInd Lits fun {$ I L}
                                    L#I
                                 end}}}
          Int2Lit:
             {NewChunk
              {List.toRecord i2l
               {List.mapInd Lits fun {$ I L}
                                    I#L
                                 end}}})
   end

   fun {Lits2Ints SetOfLiterals Literals}
      {Map Literals fun {$ Lit}
		       SetOfLiterals.Lit2Int.Lit
		    end}
   end 
   
   fun {Ints2Lits SetOfLiterals Ints}
      {Map Ints fun {$ Int}
		   SetOfLiterals.Int2Lit.Int
		end}
   end 
end


fun {CrewProb FlightData Crew}
   CabinStaff      = {Append Crew.stewards Crew.stewardesses}
   CrewSet         = {SetOfLiterals CabinStaff}
   Stewards        = {GFS.value.make
		      {Lits2Ints CrewSet Crew.stewards}}
   Stewardesses    = {GFS.value.make
		      {Lits2Ints CrewSet Crew.stewardesses}}
   FrenchSpeaking  = {GFS.value.make
		      {Lits2Ints CrewSet Crew.frenchspeaking}}
   GermanSpeaking  = {GFS.value.make
		      {Lits2Ints CrewSet Crew.germanspeaking}}
   SpanishSpeaking = {GFS.value.make
                      {Lits2Ints CrewSet Crew.spanishspeaking}}

   proc {TeamConstraint Team Flight}
      flight(no:_ crew:N stewards:NStew stewardesses:NHost
             frenchspeaking:NFrench germanspeaking:NGerman
	     spanishspeaking:NSpanish) = Flight

      C1 C2 C3 C4 C5
      [C1 C2 C3 C4 C5] ::: GFD.inf#GFD.sup
            
      S1 = {GFS.var.decl}
      S2 = {GFS.var.decl}
      S3 = {GFS.var.decl}
      S4 = {GFS.var.decl}
      S5 = {GFS.var.decl}
   in
      {GFS.card post(Team  N)}
      S1 = {GFS.intersect Team Stewards}
      S2 = {GFS.intersect Team Stewardesses}
      S3 = {GFS.intersect Team FrenchSpeaking}
      S4 = {GFS.intersect Team GermanSpeaking}
      S5 = {GFS.intersect Team SpanishSpeaking}

      {GFS.card post(S1 C1)}
      {GFS.card post(S2 C2)}
      {GFS.card post(S3 C3)}
      {GFS.card post(S4 C4)}
      {GFS.card post(S5 C5)}
      
      C1 >=: NStew
      C2 >=: NHost
      C3 >=: NFrench
      C4 >=: NGerman
      C5 >=: NSpanish
      
   end



   proc {SequencedDisjoint L}
      case L of A|B|C|T then 
         {GFS.disjoint A B}
         {GFS.disjoint A C}
         {SequencedDisjoint B|C|T}
      elseof A|B|nil then 
         {GFS.disjoint A B}
      end 
   end 
in 
   proc {$ Sol}
      Flights = {GFS.var.list.upperBound
                 {Length FlightData}
		 {Lits2Ints CrewSet CabinStaff}}
   in       
      {Map FlightData proc {$ D F}
                         {TeamConstraint F D}
                      end Flights}
                  
      {SequencedDisjoint Flights}
 
      %{GFS.distribute generic(order:naive value:min) Flights}
      {GFS.distribute naive Flights}

      Sol = {Map Flights
	     fun {$ F}
		{Ints2Lits CrewSet {GFS.monitorIn F}}
	     end}
   end 
end


Flights =
[flight(no: 1 crew:4 stewards:1 stewardesses:1
        frenchspeaking:1 spanishspeaking:1
        germanspeaking:1)
 flight(no: 2 crew:5 stewards:1 stewardesses:1
        frenchspeaking:1 spanishspeaking:1
        germanspeaking:1)
 flight(no: 3 crew:5 stewards:1 stewardesses:1
        frenchspeaking:1 spanishspeaking:1
        germanspeaking:1)
 flight(no: 4 crew:6 stewards:2 stewardesses:2
        frenchspeaking:1 spanishspeaking:1
        germanspeaking:1)
 flight(no: 5 crew:7 stewards:3 stewardesses:3
        frenchspeaking:1 spanishspeaking:1
        germanspeaking:1)
 flight(no: 6 crew:4 stewards:1 stewardesses:1
        frenchspeaking:1 spanishspeaking:1
        germanspeaking:1)
 flight(no: 7 crew:5 stewards:1 stewardesses:1
        frenchspeaking:1 spanishspeaking:1
        germanspeaking:1)
 flight(no: 8 crew:6 stewards:1 stewardesses:1
        frenchspeaking:1 spanishspeaking:1
        germanspeaking:1)
 flight(no: 9 crew:6 stewards:2 stewardesses:2
        frenchspeaking:1 spanishspeaking:1
        germanspeaking:1)
 flight(no:10 crew:7 stewards:3 stewardesses:3
        frenchspeaking:1 spanishspeaking:1
	germanspeaking:1)]

Crew =
crew(stewards:
        [tom david jeremy ron joe bill fred bob mario ed]
     stewardesses:
        [carol janet tracy marilyn carolyn cathy inez
         jean heather juliet]
     frenchspeaking:
        [inez bill jean juliet]
     germanspeaking:
        [tom jeremy mario cathy juliet]
     spanishspeaking:
	[bill fred joe mario marilyn inez heather])

{Browse {SearchOne {CrewProb Flights Crew}}}