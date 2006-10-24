%%%
%%% Authors:
%%%   Christian Schulte
%%%
%%% Copyright:
%%%   Christian Schulte, 2001
%%%
%%% Last change:
%%%   $Date: 2001/05/02 15:57:12 $ by $Author: schulte $
%%%   $Revision: 1.1 $
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

fun {MakeGolomb N}
   NN = N * N
   N2 = 2 * N
in
   proc {$ S}
      K = {GFD.tuple k N 0#FD.sup}
      D = {GFD.tuple d (NN - N) div 2 0#NN}
      fun {DIJ I J}
	 D.(((I - 1) * (N2 - I)) div 2 + J - I)
      end
   in
      S = s(k:K dm:{DIJ 1 N})
      K.1 = 0
      %K.2 :: 0#NN
      K.2 = {GFD.int 0#NN}
      for I in 1..N-1 do
	 %K.(I+1) >: K.I
	 {GFD.rel K.(I+1) GFD.rt.'>:' K.I GFD.cl.val}
	 for J in I+1..N do
	    %K.J - K.I =: {DIJ I J}
	    {GFD.sumC [1 ~1] [K.J K.I] '=:' {DIJ I J}}
	 end
      end
      %{GFD.distinctB D}
      {GFD.distinct D GFD.cl.val}
      {GFD.distribute naive K}
   end
end

proc {Better O N}
   %O.dm >: N.dm
%   {Show O.dm#N.dm}
   {GFD.rel O.dm GFD.rt.'>:' N.dm GFD.cl.val}
end

%{ExploreBest {MakeGolomb 9} Better}
%{Show {SearchBest {MakeGolomb 7} Better}}
{Show {SearchOne {MakeGolomb 7}}}


