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


%%
%% Load graphical plugin for Explorer
%%

declare
[Graphics]={Module.link [({Property.get 'oz.home'}#
			  '/examples/fd/graphics/Knights.ozf')]}
{Graphics.add}

declare
fun {Knights N}
   NN = N*N
   % The fields of the board are numbered from 1..NN
   % according to their lexicographic order, that is,
   % (1,1), (1,2), ..., (2,1), (2,2), ..., (N,N)
   %
   % Field: X x Y --> Field
   fun {Field X Y}
      (X-1)*N + Y
   end
   % Neighbours: Field --> List of fields
   fun {Neighbours F}
      X  = (F-1) mod N + 1
      Y  = (F-1) div N + 1
   in
      {FoldR [~2#~1 ~2#1 ~1#~2 ~1#2  1#~2 1#2 2#~1 2#1]
       fun {$ U#V In}
	  A = X+U
	  B = Y+V
       in
	  if A>=1 andthen A=<N andthen B>=1 andthen B=<N then
	     A + (B-1)*N | In
	  else In
	  end
       end
       nil}
   end
in
   proc {$ Solution}
      Pred  = {FD.tuple pred NN 1#NN}   % field --> field
      Succ  = {FD.tuple succ NN 1#NN}   % field --> field
      Jump  = {FD.tuple jump NN 1#NN}   % field --> jump
            = {FD.distinct}
   in
      Solution = Jump#Succ#Pred
      % there are no solutions for odd N
      N mod 2 = 0
      % tour starts as follows: (1,1), (2,3), ... 
      Jump.{Field 1 1} = 1            
      Jump.{Field 2 3} = 2
      % for every field F
      {For 1 NN 1
       proc {$ F}
	  Nbs   = {Neighbours F}
       in
	  Pred.F :: Nbs
	  Succ.F :: Nbs
	  % redundant constraint: avoid trivial cycles
	  Succ.F \=: Pred.F
          % for every neighbour G of F
	  {ForAll Nbs
	   proc {$ G}
	      (Succ.F=:G)
	      = (F=:Pred.G)
	      = (Jump.G =: {FD.modI Jump.F NN}+1)
	   end}
       end}
      {FD.distribute naive Succ} % better than ff
   end
end

{ExploreOne {Knights 8}}

/*

{Search.one.depth {Knights 18} 10 _ _}

{ExploreOne {Knights 16}}  % recomputation needed to save memory

{Graphics.delete}

*/
