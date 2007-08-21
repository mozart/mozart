%%%
%%% Authors:
%%%     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
%%%     Alberto Delgado <adelgado@cic.puj.edu.co>
%%%     Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%% Copyright:
%%%     Gustavo Gutierrez, 2006
%%%     Alberto Delgado, 2006
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

declare


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
      Root = {GFD.list N 1#N}
      D = {List.toTuple '#' Root}
      {GFD.distinct Root GFD.cl.val}
      {GFD.distinctOffset Root C1}
      {GFD.distinctOffset Root C2}
      
      %{Distribute D VarNone ValMin}
      {GFD.distribute naive Root}
      %{NaiveDistribute Root}
   end
end

S = {New Search.object script({Queens 9} rcd:4)}
{Show {S last($)}}