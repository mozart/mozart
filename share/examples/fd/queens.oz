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
fun {Queens N}
   proc {$ Row}
      L1N  = {List.number 1 N 1}      % [1 2 3 ... N]
      LM1N = {List.number ~1 ~N ~1}   % [~1 ~2 ~3 ... ~N]
   in
      Row = {FD.tuple queens N 1#N}
      {FD.distinct Row}
      {FD.distinctOffset Row L1N}
      {FD.distinctOffset Row LM1N}
      {FD.distribute generic(order:size value:mid) Row}
   end
end

{Show {SearchOne {Queens 10}}}
