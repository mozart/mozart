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
      {FD.dom 1#1000 Root}
      AA = {Square A}
      BB = {Square B}
      CC = {Square C}
      {FD.sum [AA BB] '=:' CC} % A*A + B*B =: C*C propagates much worse
      {FD.lesseq A B}
      {FD.lesseq B C}
      {FD.sumC [2 ~1] [BB CC] '>=:' 0} % redundant constraint
      {FD.distribute ff Root}
   end
end

{Show {SearchOne Pythagoras}}
