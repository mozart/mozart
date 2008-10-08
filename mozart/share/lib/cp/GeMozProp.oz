%%%
%%% Authors:
%%%     Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%%  Contributors:
%%%     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
%%% 	Andres Felipe Barco <anfelbar@univalle.edu.co>
%%%	Victor Rivera Zuniga <varivera@javerianacali.edu.co>
%%%
%%% Copyright:
%%%     Alejandro Arbelaez, 2006
%%%
%%% Last change:
%%%   $Date: 2006-10-19T01:44:35.108050Z $ by $Author: ggutierrez $
%%%   $Revision: 2 $
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

					
%%Generic propagators

proc{SumCN Iv Dvv A D}
   {Int_sumCN Iv Dvv Rt.A D}
end

proc{Sum Dv A D}
   {LinearP post(Dv A D cl:Cl.val)}
end
   
proc{SumC Iv Dv A D}
   {LinearP post(Iv Dv A D cl:Cl.val)}
end

proc{SumD Dv A D}
   {LinearP post(Dv A D cl:Cl.dom)}
end

proc{SumCD Iv Dv A D}
   {LinearP post(Iv Dv A D cl:Cl.dom)}
end


proc{SumAC Iv Dv A D} Tmp in
   Tmp = {FdDecl}
   {SumC Iv Dv A Tmp}
   {Abs post(Tmp D cl:Cl.val)}
end

proc{SumACN Iv Dv A D} Tmp in
   Tmp = {FdDecl}
   {SumCN Iv Dv A Tmp}
   {Abs post(Tmp D cl:Cl.val)}
end

%%Reified Propagators
Reified = reified(int:_ dom:_ sum:_ sumC:_ sumCN:_ sumAC:_ sumACN:_ distance:_ card:_)
%      reified(int:SI dom:NO sum:SI sumC:SI sumCN:SI sumAC:SI sumACN:NO distance:NO card:NO)

proc{SumR Dv A D1 D2}
   {LinearP post(Dv A D1 D2 cl:Cl.val)}
end

proc{SumCR Iv Dv A D1 D2}
   {LinearP post(Iv Dv A D1 D2 cl:Cl.val)}
end

proc{SumCNR Iv Dvv A D1 D2}
   {SumCNP post(Iv Dvv Rt.A D1 D2 cl:Cl.val)}
end

proc{SumACR Iv Dv A D1 D2}
   {SumACP post(Iv Dv Rt.A D1 D2)}
end

proc{SumACNR Iv Dvv A D1 D2}
   {SumACNP post(Iv Dvv Rt.A D1 D2)}
end

proc{DistanceR D1 D2 A D3 D4} Min Max in
   Min = {FdDecl}
   Max = {FdDecl}
   {MaxP post(D1 D2 Max)}
   {MinP post(D1 D2 Min)}
   {LinearP post([1 ~1] [Max Min] A D3 D4 cl:Cl.val)}
end

Reified.int = GFDP.int_reified
Reified.dom = GFDP.reified_dom
Reified.sum = SumR
Reified.sumC = SumCR
Reified.sumCN = SumCNR
Reified.sumAC = SumACR
Reified.sumACN = SumACNR
Reified.distance = DistanceR

%%Symbolic propagators
   
proc{AtLeast D Dv I}
   {CountP post(Dv I '>=:' D cl:Cl.val)}
end
   
proc{AtMost D Dv I}
   {CountP post(Dv I '=<:' D cl:Cl.val)}
end
   
proc{Exactly D Dv I}
   {CountP post(Dv I '=:' D cl:Cl.val)}
end

   
%%Miscellaneous propagators
%Disjoint = Int_disjoint
DisjointC = _

proc{Plus D1 D2 D3}
   {Sum [D1 D2] '=:' D3}
end

proc{PlusD D1 D2 D3}
   {SumD [D1 D2] '=:' D3}
end

proc{Minus D1 D2 D3}
   {Plus D2 D3 D1}
end

proc{MinusD D1 D2 D3}
   {PlusD D2 D3 D1}
end

proc{Less D1 D2}
   {RelP post(D1 '<:' D2 cl:Cl.val)}
end

proc{LessEq D1 D2}
   {RelP post(D1 '=<:' D2 cl:Cl.val)}
end

proc{Greater D1 D2}
   {RelP post(D1 '>:' D2 cl:Cl.val)}
end

proc{GreaterEq D1 D2}
   {RelP post(D1 '>=:' D2 cl:Cl.val)}
end

%%maybe this proc have to change something after we have all gecode propagators implemented
proc{Times D1 D2 D3}
   {MultP post(D1 D2 D3)}
end

proc{TimesD D1 D2 D3}
   raise unsupportedProp('gecode just support value consistence in mult propagator') end
end

proc{DivI D1 I D3}
   {DivP post(D1 I D3)}
end

proc{ModI D1 I D3}
   {ModP post(D1 I D3)}
end

proc{DivI2 D1 I D3}
   {DivP post(D1 I D3 cl:Cl.dom)}
end

proc{ModI2 D1 I D3}
   {ModP post(D1 I D3 cl:Cl.dom)}
end

proc{DistanceI D1 D2 A D3} Maximum Minimum in
   Maximum = {FdDecl}
   Minimum = {FdDecl}
   {MaxP post(D1 D2 Maximum)}
   {MinP post(D1 D2 Minimum)}
   {LinearP post([1 ~1] [Maximum Minimum] A D3 cl:Cl.val)}
end

proc {Power D1 I D2}
   {PowerP post(D1 I D2 cl:Cl.val)}
end


%%Assigning Values


