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

%%% crypto arithmetic
%%% A/BC + D/EF + G/HI = 1
%%% letters must be pairwise digits between 1 and 9
%%%
%%% Solution: 9/12 + 5/34 + 7/68 = 1
%%% Eliminate symmetry by A/BC >= D/EF >= G/HI
%%% Yields redundant 2 constraints:  3(A/BC) >= 1,  3(G/HI) =< 1

declare
proc {Fraction Root}
   sol(a:A b:B c:C d:D e:E f:F g:G h:H i:I) = !Root
   BC = {FD.decl}
   EF = {FD.decl}
   HI = {FD.decl}
in
   %Root ::: 1#9
   {FD.dom 1#9 Root}
   {FD.distinct Root}
   %BC =: 10*B + C
   {FD.sumC [10 1] [B C] '=:' BC}
   %EF =: 10*E + F
   {FD.sumC [10 1] [E F] '=:' EF}
   %HI =: 10*H + I
   {FD.sumC [10 1] [H I] '=:' HI}
   %A*EF*HI + D*BC*HI + G*BC*EF =: BC*EF*HI
   {FD.sumCN [1 1 1 ~1] [[A EF HI] [D BC HI] [G BC EF] [BC EF HI]] '=:' 0}
   %% impose order
   %A*EF >=: D*BC
   {FD.sumCN [1 ~1] [[A EF] [D BC]] '>=:' 0}
   %D*HI >=: G*EF
   {FD.sumCN [1 ~1] [[D HI] [G EF]] '>=:' 0}
   %% redundant constraints
   %3*A >=: BC
   {FD.sumC [3] [A] '>:' BC}
   %3*G =<: HI
   {FD.sumC [3] [G] '<:' HI}

   {FD.distribute split Root}
end

{Show {SearchAll Fraction}}
