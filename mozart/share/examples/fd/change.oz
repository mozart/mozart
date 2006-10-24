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
fun {ChangeMoney BillAndCoins Amount}
   Available    = {Record.map BillAndCoins fun {$ A#_} A end}
   Denomination = {Record.map BillAndCoins fun {$ _#D} D end}
   NbDenoms     = {Width Denomination}
in
   proc {$ Change}
      {FD.tuple change NbDenoms 0#Amount Change}
      {For 1 NbDenoms 1 proc {$ I}
			   Change.I =<: Available.I
			end}
      {FD.sumC Denomination Change '=:' Amount}
      {FD.distribute generic(order:naive value:max) Change}
   end
end

BillAndCoins = r(6#100  8#25  10#10  1#5  5#1)

{ExploreOne {ChangeMoney BillAndCoins 142}}

