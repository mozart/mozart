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
proc {Money Root}
   S E N D M O R Y
in
   Root = [S E N D M O R Y]
   {FD.dom 0#9 Root}
   {FD.distinct Root}
   {FD.sumC
    [1000 100 10 1 1000 100 10 1 ~10000 ~1000 ~100 ~10 ~1]
    [S E N D M O
     R E M O N E Y]
    '=:' 0}
   
   {FD.sum [S] '\\=:' 0}
   {FD.sum [M] '\\=:' 0}

   {FD.distribute ff Root}
end

{Show {SearchOne Money}}
