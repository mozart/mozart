% A kid goes into a grocery store and
% buys four items.  The cashier charges
% $7.11 , the kid pays and is about to
% leave when the cashier calls the kid
% back, and says "Hold on, I multiplied
% the four items instead of adding
% them. I'll try again. Hah!, with
% adding them the price still comes to
% $7.11"
%
% Question: What were the prices of the
% four items?

declare
proc {Grocery Root}
   A#B#C#D = Root
   S       = 711
in
   Root ::: 0#S
   A*B*C*D =: S*100*100*100
   A+B+C+D =: S
   % eliminate symmetries
   A =: 79*{FD.decl}  % 79 is prime factor of S=711
   B =<: C
   C =<: D 
   {FD.distribute generic(value:splitMax) Root}
end

{ExploreAll Grocery}
