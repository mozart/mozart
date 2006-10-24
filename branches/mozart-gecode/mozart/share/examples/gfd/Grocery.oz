%%%
%%% Authors:
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Gert Smolka, 1998
%%%
%%% Last change:
%%%   $Date: 1999/01/18 21:56:04 $ by $Author: schulte $
%%%   $Revision: 1.2 $
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

%%% A kid goes into a grocery store and
%%% buys four items.  The cashier charges
%%% $7.11 , the kid pays and is about to
%%% leave when the cashier calls the kid
%%% back, and says "Hold on, I multiplied
%%% the four items instead of adding
%%% them. I'll try again. Hah!, with
%%% adding them the price still comes to
%%% $7.11"
%%%
%%% Question: What were the prices of the
%%% four items?

\switch +geoz
declare
proc {Grocery Root}
   A#B#C#D = Root
   S       = 711
in
   Root ::: 0#S
   A*B*C*D =: S*100*100*100
   A+B+C+D =: S
   %% eliminate symmetries
   A =: 79*{GFD.decl}  % 79 is prime factor of S=711
   B =<: C
   C =<: D 
   {GFD.distribute ff Root}
end

%{ExploreOne Grocery}
{Show {SearchOne Grocery}}
%%{Show {SearchAll Grocery}}
