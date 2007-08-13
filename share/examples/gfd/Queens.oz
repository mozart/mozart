/*
 *  Main authors:
 *     Alejandro Arbelaez: <aarbelaez@puj.edu.co>
 *
 *
 *  Contributing authors:
 *
 *
 *  Copyright:
 *     Alejandro Arbelaez, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

declare


fun {SOne Script}
  try {EOne {Space.new Script}} nil
  catch solution(S) then [{Space.merge S}] end
end

proc{Print S} {Space.inject S proc{$ R} {Browse 'SS'#R} end} end

proc {EOne S}
  case {Space.ask S}
  of succeeded then {Print S} raise solution(S) end
  [] failed then skip
  [] branch([A B]) then C={Space.clone S} in
     {Space.commitB S A} {EOne S}
     {Space.commitB C B} {EOne C}
  end
end

fun{EAll S}
   case {Space.ask S}
   of succeeded then [{Space.merge S}]
   [] failed then nil
   [] branch([A B]) then C = {Space.clone S} in
      {Space.commitB S A} {Space.commitB C B}
      {Append {EAll S} {EAll C}}
   end    
end

%% a naive distributor that takes advantage of batch recomputation
proc {NaiveDistribute Xs}
  V = if {IsList Xs} then {List.toTuple '#' Xs} else Xs end
  proc {Distribute L}
     case {Space.getChoice}
     of I#D  then
	%{Show 'I#D'#I#D}
	case D
	of compl(M) then V.I \=: M
	[] M then V.I =: M
	end
	{Distribute L}
     [] nil then
        case {List.dropWhile L fun {$ I#X} {IsDet X} end}
        of nil then
	   skip
        [] L1 then
	   I#X|_=L1
	   %{Inspect 'I'#I}
           M={GFD.reflect.min X}
        in
           {Space.branch [I#M I#compl(M)]}
           {Distribute L1}
        end
     end
  end
in
  {Distribute {Record.toListInd V}}
end

proc{Foo R} A B in R = [A B]
   R:::0#2
   {GFD.distinct R GFD.cl.dom}
   {NaiveDistribute R}
end


fun{Queens N}
   proc{$ Root}
      C1 = {List.number 1 N 1}
      C2 = {List.number ~1 ~N ~1}
      D
   in
      Root = {GFD.intVarList N 1#N}
      D = {List.toTuple '#' Root}
      {GFD.distinct Root GFD.cl.val}
      {GFD.distinctOffset Root C1}
      {GFD.distinctOffset Root C2}
      
      %{Distribute D VarNone ValMin}
      {GFD.distribute naive Root}
      %{NaiveDistribute Root}
   end
end

%{Show 'Queens'#{SOne {Queens 9}}}
%{Inspect 'QueensAll'#{EAll {Space.new {Queens 6}}}}
%{Show 'QueensAll'#{EAll {Space.new {Queens 6}}}}

S = {New Search.object script({Queens 9} rcd:4)}
{Show {S last($)}}