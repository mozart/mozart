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
proc{Donald Root}
   D O N A L
   G E R B T
   RootArgs
in
   Root = [D O N A L G E R B T]
   RootArgs = [100000 10000 1000 100 10 1
	       100000 10000 1000 100 10 1
	       ~100000 ~10000 ~1000 ~100 ~10 ~1]
   {FD.dom 0#9 Root}
   {FD.sum [D] '\\=:' 0}
   {FD.sum [G] '\\=:' 0}
   {FD.sum [R] '\\=:' 0}
   {FD.sumC RootArgs
    [D O N A L D
     G E R A L D
     R O B E R T]
    '=:' 0}
   {FD.distinct Root}
   {FD.distribute ff Root}
end

{Show {SearchOne Donald}}
