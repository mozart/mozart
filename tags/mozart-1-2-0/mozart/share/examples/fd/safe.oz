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

%%% Professor Smart has coded his safe combination as follows:

declare
proc {Safe C}
   {FD.tuple code 9 1#9 C}
   {FD.distinct C}
   {For 1 9 1 proc {$ I} C.I \=: I end}
   C.4 - C.6 =: C.7
   C.1 * C.2 * C.3 =: C.8 + C.9
   C.2 + C.3 + C.6 <: C.8
   C.9 <: C.8
   {FD.distribute ff C}
end

{ExploreAll Safe}
