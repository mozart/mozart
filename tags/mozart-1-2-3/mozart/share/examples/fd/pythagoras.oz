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
local
   proc {Square X S}
      {FD.times X X S}     % exploits coreference
   end
in
   proc {Pythagoras Root}
      [A B C] = Root
      AA BB CC
   in
      Root ::: 1#1000
      AA = {Square A}
      BB = {Square B}
      CC = {Square C}
      AA + BB =: CC           % A*A + B*B =: C*C propagates much worse
      A =<: B
      B =<: C
      2*BB >=: CC             % redundant constraint
      {FD.distribute ff Root}
   end
end

{ExploreOne Pythagoras}

/*
{SearchAll Pythagoras _}
*/