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
			  '/examples/fd/graphics/Queens.ozf')]}
{Graphics.add}



declare
fun {Queens N}
   proc {$ Row}
      L1N  = {List.number 1 N 1}      % [1 2 3 ... N]
      LM1N = {List.number ~1 ~N ~1}   % [~1 ~2 ~3 ... ~N]
   in
      Row = {FD.tuple queens N 1#N}
      {FD.distinct Row}
      {FD.distinctOffset Row L1N}
      {FD.distinctOffset Row LM1N}
      {FD.distribute generic(order:size value:mid) Row}
   end
end

{ExploreOne {Queens 33}}  % tough



/*

declare
fun {Order X Y}
   SX={FD.reflect.size X}
   SY={FD.reflect.size Y}
in
   SX < SY orelse
   SX == SY andthen {FD.reflect.min X} < {FD.reflect.min Y}
end

fun {Queens N}
   proc {$ Row}
      L1N  = {List.number 1 N 1}    % [1 2 3 ... N]
      LM1N = {List.number ~1 ~N ~1} % [~1 ~2 ~3 ... ~N]
   in
      Row = {FD.tuple queens N 1#N}
      {FD.distinct Row}
      {FD.distinctOffset Row L1N}
      {FD.distinctOffset Row LM1N}
      {FD.distribute generic(order:Order) Row}
   end
end

% works perfect for large n

{ExploreOne {Queens 913}}

% doesn't work so well for small n

{ExploreOne {Queens 33}}

% timing with recomputation

{Search.one.depth {Queens 913} 4000 _ _}

{Graphics.delete}

*/