%%%
%%% Authors:
%%%   Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
%%%   Alberto Delgado <adelgado@cic.puj.edu.co>
%%%   Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%% Copyright:
%%%   Gustavo Gutierrez, 2006
%%%   Alberto Delgado, 2006
%%%   Alejandro Arbelaez, 2006
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

%%% Adapted from a finite domain example in Mozart-Oz version 1.3.2 by 
%%% Christian Schulte, 2001.

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
      K.1 =: 0
      K.2 =: {GFD.int 0#NN}
      for I in 1..N-1 do
	 %% K.(I+1) >: K.I
	 {GFD.relP post(K.(I+1) GFD.rt.'>:' K.I cl:GFD.cl.val)}
	 for J in I+1..N do
	    %% K.J - K.I =: {DIJ I J}
	    {GFD.sumC [1 ~1] [K.J K.I] '=:' {DIJ I J}}
	 end
      end
      {GFD.distinctP post(D cl:GFD.cl.val)}
      {GFD.distributeBR naive K}
   end
end

proc {Better O N}
   %%O.dm >: N.dm
   %%{Show O.dm#N.dm}
   {GFD.rel O.dm GFD.rt.'>:' N.dm GFD.cl.val}
end

%{Show {SearchBest {MakeGolomb 7} Better}}
{Show {SearchOne {MakeGolomb 7}}}


