%%%
%%% Authors:
%%%     Gustavo A. Gomez Farhat <gafarhat@univalle.edu.co>
%%%
%%% Contributors:
%%%     Andres Felipe Barco <anfelbar@univalle.edu.co>
%%%
%%% Copyright:
%%%     Gustavo A. Gomez Farhat, 2008
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

%%% Adapted from an example in "Finite Set Constraints in Oz, Tobias and
%%% Martin Muller, 1997"

declare

fun {Steiner N}
   proc {$ Triples}
      N1 = N+1
      N1N1 = N1*N1
   in
      if N mod 6 == 1 orelse N mod 6 == 3 then
         % create list of triples which model set of cardinality 3
	 Triples = {MakeList N*(N-1) div 6}
	 {ForAll Triples proc{$ T} T = {GFD.list 3 1#N} end}
         % triple elements must be different
	 {ForAll Triples proc{$ T} {GFD.distinctP post(T)} end}
	 % all pairs in two different triples must be different

	 {ForAllTail Triples proc{$ [T11 T12 T13]|Tr}
				{ForAll Tr
				 proc{$ [T21 T22 T23]}
				    
				    B1 = {GBD.decl}
				    {GFD.relP post(T11 GFD.rt.'=:' T21 B1)}

				    B2 = {GBD.decl}
				    {GFD.relP post(T11 GFD.rt.'=:' T22 B2)}
				    
				    B3 = {GBD.decl}
				    {GFD.relP post(T11 GFD.rt.'=:' T23 B3)}

				    B4 = {GBD.decl}
				    {GFD.relP post(T12 GFD.rt.'=:' T21 B4)}

				    B5 = {GBD.decl}
				    {GFD.relP post(T12 GFD.rt.'=:' T22 B5)}

				    B6 = {GBD.decl}
				    {GFD.relP post(T12 GFD.rt.'=:' T23 B6)}

				    B7 = {GBD.decl}
				    {GFD.relP post(T13 GFD.rt.'=:' T21 B7)}

				    B8 = {GBD.decl}
				    {GFD.relP post(T13 GFD.rt.'=:' T22 B8)}

				    B9 = {GBD.decl}
				    {GFD.relP post(T13 GFD.rt.'=:' T23 B9)}
				    
				 in
				    {GBD.linearP
				     post([B1 B2 B3 B4 B5 B6 B7 B8 B9]
					  GBD.rt.'=<:' 1)}
				 end}
			     end}
         % order triple elements
	 {ForAll Triples proc{$ [T1 T2 T3]} T1<:T2 T2<:T3 end}
         % impose order on triples
	 {ForAllTail Triples proc{$ [T11 T12 T13]|Tr}
				case Tr of nil then skip
				[] [T21 T22 T23]|_
				then
				   {GFD.linearP
				    post([N1N1 N1 1 ~N1N1 ~N1 ~1]
					 [T11 T12 T13 T21 T22 T23]
					 GFD.rt.'<:'
					 0)}
				end
			     end}
	 
         % create choice points
	 {GFD.distribute ff {Flatten Triples}}
      else fail
      end
   end
end

{Show {SearchOne {Steiner 7}}}
