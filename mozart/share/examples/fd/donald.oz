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
proc {Donald Root}
   sol(a:A b:B d:D e:E g:G l:L n:N  o:O r:R t:T) = Root
in
   Root ::: 0#9
   {FD.distinct Root}
   D\=:0  R\=:0  G\=:0
      100000*D + 10000*O + 1000*N + 100*A + 10*L + D
   +  100000*G + 10000*E + 1000*R + 100*A + 10*L + D
   =: 100000*R + 10000*O + 1000*B + 100*E + 10*R + T
   {FD.distribute ff Root}
end

{ExploreAll Donald}