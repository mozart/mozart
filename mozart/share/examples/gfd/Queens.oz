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

GenSelVar = map(naive: fun {$ _ _} false end )
GenSelFil = map(undet: fun {$ X} {GFD.reflect.size X}>1 end)
SelVal = map(min: GFD.reflect.min)

proc {Distribute generic(selvar:Var filter:Fil value:Val) Xs}
  V = if {IsList Xs} then {List.toTuple '#' Xs} else Xs end
   proc {Distribute L}
     case {Space.getChoice}
     of I#D then
	case D
	of compl(M) then V.I \=: M
	[] M then V.I =: M
	end
	{Distribute L}
     [] nil then
	LFil = {List.filter L fun {$ I#X} {Fil X} end}
     in
	if LFil \= nil then
	   LSel = {List.foldL LFil fun {$ I#X Acc} if {Var X Acc} then Acc else I#X end end LFil.1}
	   I#X = LSel
           M={Val X}
	in
	   {Space.branch [I#M I#compl(M)]}
           {Distribute LFil}
        end
     end
  end
in
  {Distribute {Record.toListInd V}}
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
      
      %{GFD.distribute naive Root}
      {Distribute generic(selvar:GenSelVar.naive filter:GenSelFil.undet value:SelVal.min) Root}
   end
end

{Show {SearchAll {Queens 5}}}
