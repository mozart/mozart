% Given N, find S=(X_0,...,X_N-1) such that
%  - X_i in 0..N-1
%  - i occurs X_i-times in S

declare
fun {MagicSequence N}
   Cs = {List.number ~1 N-2 1}
in
   proc {$ S}
      {FD.tuple sequence N 0#N-1 S}
      {For 0 N-1 1
       proc {$ I} {FD.exactly S.(I+1) S I} end}
      {FD.sum S '=:' N}   % redundant
      % redundant: sum (i-1)*X_i = 0 (since  sum i*X_i = sum X_i)
      {FD.sumC Cs S '=:' 0}
      %
      {FD.distribute ff S}
   end
end


{ExploreAll {MagicSequence 17}}



% Conjecture: for N>5 is X_0 = N-3
% case N>5 then S.1=N-3 else skip end

