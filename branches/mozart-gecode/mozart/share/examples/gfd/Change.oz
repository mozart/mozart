%%%
%%% Authors:
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Gert Smolka, 1998
%%%
%%% Last change:
%%%   $Date: 1999/06/09 14:30:43 $ by $Author: schulte $
%%%   $Revision: 1.3 $
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
      {GFD.tuple change NbDenoms 0#Amount Change}
      {For 1 NbDenoms 1 proc {$ I}
                           %Change.I =<: Available.I
			   {GFD.relP post(Change.I GFD.rt.'=<:' Available.I cl:GFD.cl.val)}
                        end}
      {GFD.sumC Denomination Change '=:' Amount}
      {GFD.distributeBR generic(order:naive value:max) Change}
   end
end

BillAndCoins = r(6#100  8#25  10#10  1#5  5#1)

%{ExploreAll {ChangeMoney BillAndCoins 142}}
{Show {SearchAll {ChangeMoney BillAndCoins 142}}}