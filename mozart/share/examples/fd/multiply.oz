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
proc {Solution Sol}
   A B C D E F G H I J K L M N O P Q R S T
   N1 N2 R1 R2 R3
in
   %% the solution consists of digits
   Sol = [A B C D E F G H I J K L M N O P Q R S T]
   %Sol ::: 0#9
   {FD.dom 0#9 Sol}
   %% each digit occurs exactly twice
   {ForAll {List.number 0 9 1}
    proc {$ I} {FD.exactly 2 Sol I} end}
   %% no leading zeros
   %[A D G J M P] ::: compl(0)
   {FD.dom compl(0) [A D G J M P]}
   %% the 2 operands and 3 intermediate results
   %[N1 N2 R1 R2 R3]:::1#999
   {FD.dom 1#999 [N1 N2 R1 R2 R3]}
   %N1 =: 100*A+10*B+C
   {FD.sumC [100 10 1 ~1] [A B C N1] '=:' 0}
   %N2 =: 100*D+10*E+F
   {FD.sumC [100 10 1 ~1] [D E F N2] '=:' 0}
   %R1 =: 100*G+10*H+I
   {FD.sumC [100 10 1 ~1] [G H I R1] '=:' 0}
   %R2 =: 100*J+10*K+L
   {FD.sumC [100 10 1 ~1] [J K L R2] '=:' 0}
   %R3 =: 100*M+10*N+O
   {FD.sumC [100 10 1 ~1] [M N O R3] '=:' 0}
   %% compute intermediate results (method 1)
   %F*N1 =: R1
   {FD.sumCN [1 ~1] [[F N1] [R1]] '=:' 0}
   %E*N1 =: R2
   {FD.sumCN [1 ~1] [[E N1] [R2]] '=:' 0}
   %D*N1 =: R3
   {FD.sumCN [1 ~1] [[D N1] [R3]] '=:' 0}
   %% compute intermediate results (method 2)
   local
      proc {Mul I [X1 X2 X3] [Y1 Y2 Y3]}
         C1 C2
      in
	 %[C1 C2]:::0#9
	 {FD.dom 0#9 [C1 C2]}
	 %I*X3      =: Y3 + 10*C1
	 {FD.sumCN [1 10 ~1] [[Y3] [C1] [I X3]] '=:' 0}
	 %I*X2 + C1 =: Y2 + 10*C2
	 {FD.sumCN [1 10 ~1 ~1] [[Y2] [C2] [I X2] [C1]] '=:' 0}
	 %I*X1 + C2 =: Y1
	 {FD.sumCN [1 1 ~1] [[I X1] [C2] [Y1]] '=:' 0}
      end
   in
      {Mul F [A B C] [G H I]}
      {Mul E [A B C] [J K L]}
      {Mul D [A B C] [M N O]}
   end
   %% add up intermediate results (method 1)
   %100*R3+10*R2+R1 =: 10000*P+1000*Q+100*R+10*S+T
   {FD.sumC [100 10 1 ~10000 ~1000 ~100 ~10 ~1] [R3 R2 R1 P Q R S T] '=:' 0}
   %% add up intermediate results (method 2)
   local C1 C2 C3 in
      %[C1 C2 C3] ::: [0 1 2]
      {FD.dom [0 1 2] [C1 C2 C3]}
      I=T
      %H+L      =: S + 10*C1
      {FD.sumC [1 1 ~1 ~10] [H L S C1] '=:' 0}
      %G+K+O+C1 =: R + 10*C2
      {FD.sumC [1 1 1 1 ~1 ~10] [G K O C1 R C2] '=:' 0}
      %J+N  +C2 =: Q + 10*C3
      {FD.sumC [1 1 1 ~1 ~10] [J N C2 Q C3] '=:' 0}
      %M    +C3 =: P
      {FD.sumC [1 1 ~1] [M C3 P] '=:' 0}
   end
   %% break symmetry
   %N1 =<: N2
   {FD.lesseq N1 N2}
   %% reduce search space
   %F\=:1 %else C=I=T
   {FD.sum [F] '\\=:' 1}
   %F\=:0 %else F=I=T=0
   {FD.sum [F] '\\=:' 0}
   %C\=:1 %else F=I=T
   {FD.sum [C] '\\=:' 1}
   %C\=:0 %else C=I=T=0
   {FD.sum [C] '\\=:' 0}
   %% distribution strategy
   {FD.distribute ff Sol}
end

{Show {SearchAll Solution}}
