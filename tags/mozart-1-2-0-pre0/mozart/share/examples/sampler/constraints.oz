%%%
%%% Authors:
%%%   Tobias Müller <tmueller@ps.uni-sb.de>
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Tobias Müller, 1998
%%%   Christian Schulte, 1998
%%%   Gert Smolka, 1998
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

%%%
%%% Constraints & propagators
%%%

declare L X Y Z in L = [X Y Z]

{Inspect L}

{FD.dom 1#10 L}

2 * Y =: Z

X <: Y
 
Z <: 7

X \=: 1





%%%
%%% Propagation & enumeration
%%%

declare
proc {Problem Sol}
   X Y Z
in
   Sol = s(x:X y:Y z:Z) = {FD.dom 1#7}
   X + Y =: 3*Z
   X - Y =: Z
   {FD.distribute naive Sol}
end

{Inspect {SearchOne Problem}}

{Inspect {SearchAll Problem}}
     
{Explorer.object script(Problem)}








%%%
%%% Send More Money
%%%

declare
proc {Money Sol}
   S E N D M O R Y
in
   Sol = s(s:S e:E n:N d:D m:M o:O r:R y:Y)
   Sol ::: 0#9
   {FD.distinct Sol}
   S \=: 0
   M \=: 0
                1000*S + 100*E + 10*N + D
              + 1000*M + 100*O + 10*R + E
   =: 10000*M + 1000*O + 100*N + 10*E + Y
   {FD.distribute ff Sol}
end

{Explorer.object script(Money)}





%%%
%%% Magic Square
%%%
declare
proc {MagicSquare ?Sol}
   s(square:Square sum:Sum) = Sol
   [N11 N12 N13 N21 N22 N23 N31 N32 N33] = Square
in
   Sum = {FD.decl}
   Square ::: 1#9
   {FD.distinct Square}
   N11 + N12 + N13 =: Sum
   N21 + N22 + N23 =: Sum
   N31 + N32 + N33 =: Sum
   N11 + N21 + N31 =: Sum
   N12 + N22 + N32 =: Sum
   N13 + N23 + N33 =: Sum
   N11 + N22 + N33 =: Sum
   N31 + N22 + N13 =: Sum
   {FD.distribute ff Square}
end

{ExploreOne MagicSquare}





%%%
%%% A smarter solution...
%%%

declare
proc {SmartSquare ?Sol}
   thread
      9 * (9 + 1) div 2 =: 3 * Sol.sum
   end
   {MagicSquare Sol}
end

{ExploreOne SmartSquare}














%%%
%%% Aligning for a photo
%%%
declare
Persons = [alice bert chris deb evan]
Prefs   = [alice#chris bert#evan chris#deb
	   chris#evan deb#alice deb#evan
	   evan#alice evan#bert]
proc {PhotoConstraints Sol}
   Pos   = {FD.record pos Persons
	              1#{Length Persons}}
   Sat   = {Map Prefs
	    fun {$ A#B}
	       (Pos.A+1 =: Pos.B) +
	       (Pos.A-1 =: Pos.B) =: 1
	    end}
   Total = {FD.int 0#{Length Prefs}}
in
   {FD.distinct Pos}
   {FD.sum Sat '=:' Total}
   Sol = s(pos:Pos total:Total sat:Sat)
end


declare
proc {PhotoNaive Sol}
   {PhotoConstraints Sol}
   {FD.distribute naive Sol.pos}
end

declare
proc {MaxSat O N}
   O.total <: N.total
end

{Explorer.object script(PhotoNaive MaxSat)}













%%%
%%% Graphical output
%%%
\insert constraints/draw-photo.oz

{Explorer.object add(information DrawPhoto)}


















%%%
%%% Use a better enumeration
%%%
declare
proc {PhotoBetter Sol}
   {PhotoConstraints Sol}
   {FD.distribute generic(order:nbSusps) Sol.pos}
end

{ExploreBest PhotoBetter MaxSat}














%%%
%%% Removing symmetries
%%%

declare
proc {Photo Sol}
   thread
      Sol.pos.alice >: Sol.pos.bert
   end
   {PhotoBetter Sol}
end

{ExploreBest Photo MaxSat}











%%%
%%% Bridge
%%%

%% bridge specification
\insert constraints/bridge.oz

%% Scheduling compiler
\insert constraints/scheduling-compiler.oz


{Explorer.object script({Compile Bridge}
		 proc {$ O N}
		    O.pe >: N.pe
		 end)}


















%%%
%%% Production scheduling
%%%
\insert constraints/abz6.oz

{ExploreBest {Compile ABZ6} proc {$ O N}
			       O.pe >: N.pe
			    end}












%%%
%%% Show the bridge schedule
%%%

\insert constraints/animate-bridge.oz



















%%%
%%% Create Hamming Code 
%%%
\insert constraints/hamming.oz

% code 16 symbols in 7-bit words with minimal hamming distance of 2

{ExploreOne {Hamming 7 2 16}}

















%%%
%%% Allocating cabin crews
%%%

\insert constraints/crew.oz

declare
Flights = [flight(no: 1 crew:4 stewards:1 stewardesses:1
		  frenchspeaking:1 spanishspeaking:1 germanspeaking:1)
	   flight(no: 2 crew:5 stewards:1 stewardesses:1
		  frenchspeaking:1 spanishspeaking:1 germanspeaking:1)
	   flight(no: 3 crew:5 stewards:1 stewardesses:1
		  frenchspeaking:1 spanishspeaking:1 germanspeaking:1)
	   flight(no: 4 crew:6 stewards:2 stewardesses:2
		  frenchspeaking:1 spanishspeaking:1 germanspeaking:1)
	   flight(no: 5 crew:7 stewards:3 stewardesses:3
		  frenchspeaking:1 spanishspeaking:1 germanspeaking:1)
	   flight(no: 6 crew:4 stewards:1 stewardesses:1
		  frenchspeaking:1 spanishspeaking:1 germanspeaking:1)
	   flight(no: 7 crew:5 stewards:1 stewardesses:1
		  frenchspeaking:1 spanishspeaking:1 germanspeaking:1)
	   flight(no: 8 crew:6 stewards:1 stewardesses:1
		  frenchspeaking:1 spanishspeaking:1 germanspeaking:1)
	   flight(no: 9 crew:6 stewards:2 stewardesses:2
		  frenchspeaking:1 spanishspeaking:1 germanspeaking:1)
	   flight(no:10 crew:7 stewards:3 stewardesses:3
		  frenchspeaking:1 spanishspeaking:1 germanspeaking:1)
	  ]

declare 
Crew = crew(stewards:
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


{ExploreOne {CrewProb Flights Crew}}
