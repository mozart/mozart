
%%
%% Load graphical plugin for Explorer
%%

\feed 'examples/fd/queens/graphics.oz'

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

declare Kill in
{Search.one.depth {Queens 913} 4000 _ _}

*/