%%%
%%% Authors:
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
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

declare
fun {MagicSquare N}
   NN  = N*N
   L1N = {List.number 1 N 1}  % [1 2 3 ... N]
in
   proc {$ Square}
      fun {Field I J}
	 Square.((I-1)*N + J)
      end
      proc {Assert F}
         %% {F 1} + {F 2} + ... + {F N} =: Sum
	 {FD.sum {Map L1N F} '=:' Sum}
      end
      Sum = {FD.decl} 
   in
      {FD.tuple square NN 1#NN Square}
      {FD.distinct Square}
      %% Diagonals
      {Assert fun {$ I} {Field I I} end}
      {Assert fun {$ I} {Field I N+1-I} end}
      %% Columns
      {For 1 N 1
       proc {$ I} {Assert fun {$ J} {Field I J} end} end}
      %% Rows
      {For 1 N 1
       proc {$ J} {Assert fun {$ I} {Field I J} end} end}
      %% Eliminate symmetries
      /* {Field 1 1} <: {Field N N}
      {Field N 1} <: {Field 1 N}
      {Field 1 1} <: {Field N 1} */
      %% Redundant: sum of all fields = (number rows) * Sum
      NN*(NN+1) div 2 =: N*Sum
      %%
      {FD.distribute split Square}
   end
end


{ExploreOne {MagicSquare 3}}


