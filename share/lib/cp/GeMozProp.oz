%%%
%%% Authors:
%%%     Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%%  Contributors:
%%%     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
%%% 	Andres Felipe Barco <anfelbar@univalle.edu.co>
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
   {LinearP post(Dv Rt.A D cl:Cl.val)}
end
   
proc{SumC Iv Dv A D}
   {LinearP post(Iv Dv Rt.A D cl:Cl.val)}
end

proc{SumD Dv A D}
   {LinearP post(Dv Rt.A D cl:Cl.dom)}
end

proc{SumCD Iv Dv A D}
   {LinearP post(Iv Dv Rt.A D cl:Cl.dom)}
end


proc{SumAC Iv Dv A D SP} Tmp in
   Tmp = {FdDecl}
   {SumC Iv Dv A Tmp}
   {Abs Tmp D Cl.val}
end

proc{SumACN Iv Dv A D SP} Tmp in
   Tmp = {FdDecl}
   {SumCN Iv Dv A Tmp}
   {Abs Tmp D Cl.val}
end

%%Reified Propagators
Reified = reified(sum:_ sumC:_ sumCN:_ int:_)

%%FD.reified.sumC +Iv *Dv +A *D1 D2}
proc{SumR Dv A D1 D2}
   {LinearP post(Dv Rt.A D1 D2 cl:Cl.val)}
end

proc{SumCR Iv Dv A D1 D2}
   {LinearP post(Iv Dv Rt.A D1 D2 cl:Cl.val)}
end

Reified.sum = SumR
Reified.sumC = SumCR

%%Symbolic propagators
   
proc{AtLeast D Dv I}
   {CountP post(Dv D Rt.'=<:' I cl:Cl.val)}
end
   
proc{AtMost D Dv I}
   {CountP post(Dv D Rt.'>=:' I cl:Cl.val)}
end
   
proc{Exactly D Dv I}
   {CountP post(Dv D Rt.'=:' I cl:Cl.val)}
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
   {RelP post(D1 Rt.'<:' D2 cl:Cl.val)}
end

proc{LessEq D1 D2}
   {RelP post(D1 Rt.'=<:' D2 cl:Cl.val)}
end

proc{Greater D1 D2}
   {RelP post(D1 Rt.'>:' D2 cl:Cl.val)}
end

proc{GreaterEq D1 D2}
   {RelP post(D1 Rt.'>=:' D2 cl:Cl.val)}
end

%%maybe this proc have to change something after we have all gecode propagators implemented
proc{Times D1 D2 D3}
   {MultP post(D1 D2 D3)}
end

proc{TimesD D1 D2 D3}
   raise unsupportedProp('gecode just support value consistence in mult propagator') end
end

proc{DivI D1 I D3}
   {Wait I}
   {MultP post(D3 D1 I)}
end

%%Assigning Values

%%Reified Propagators
%%   proc{IntR Spec D1 D2}
%%      {Int_reified_int Spec D1 D2}
%%   end
%%   Reified.int = IntR

%%Pending ....
Reified.int = GFDP.int_reified 


