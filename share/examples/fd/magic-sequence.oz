%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
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

%%% Given N, find S=(X_0,...,X_N-1) such that
%%%  - X_i in 0..N-1
%%%  - i occurs X_i-times in S

declare
fun {MagicSequence N}
   Cs = {List.number ~1 N-2 1}
in
   proc {$ S}
      {FD.tuple sequence N 0#N-1 S}
      {For 0 N-1 1
       proc {$ I} {FD.exactly S.(I+1) S I} end}
      {FD.sum S '=:' N}   % redundant
      %% redundant: sum (i-1)*X_i = 0 (since  sum i*X_i = sum X_i)
      {FD.sumC Cs S '=:' 0}
      {FD.distribute ff S}
      %% Also try the following
      %% {FD.distribute splitMin S}
   end
end


{ExploreAll {MagicSequence 17}}



%% Conjecture: for N>5 is X_0 = N-3
%% case N>5 then S.1=N-3 else skip end

