%%%
%%% Authors:
%%%   Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
%%%   Alberto Delgado <adelgado@cic.puj.edu.co>
%%%   Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%% Copyright:
%%%   Gustavo Gutierrez, 2006
%%%   Alberto Delgado, 2006
%%%   Alejandro Arbelaez, 2006
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

%%% Adapted from a finite domain example in Mozart-Oz version 1.3.2 by 
%%% Gert Smolka, 1998.

declare

fun {SelectSize L}
   fun {SelectSizeAux L Lst}
      case L of nil then Lst
      [] (A#X)|Xs then
	 if {GFD.reflect.size X} < {GFD.reflect.size Lst.1} then
	    {SelectSizeAux Xs A#X}
	 else
	    {SelectSizeAux Xs Lst}
	 end
      end
   end
in
   {SelectSizeAux L L.1}
end

   
   
proc {NaiveDistribute Xs}
   V = if {IsList Xs} then {List.toTuple '#' Xs} else Xs end
   proc {Distribute L}
      case {Space.getChoice}
      of I#D  then 
	 case D
	 of eq(M) then {GFD.relP post(V.I GFD.rt.'=:' M)}
	 [] neq(M) then {GFD.relP post(V.I GFD.rt.'\\=:' M)}
	 %[] lt(M) then V.I =: M	   
	 end
	 {Distribute L}
      [] nil then
	 case {List.dropWhile L fun {$ I#X} {IsDet X} end}
	 of nil then
	    skip
	 [] L1 then		  
	    I#X = {SelectSize L1}
	    
		  %I#X|_=L1
	    M={GFD.reflect.med X}
	 in
	    %{Show sel(I#M#Xs)}
	    %{Space.branch [I#eq(M) I#lt(M) I#gt(M) ]}
	    {Space.branch [I#eq(M) I#neq(M)]}
	    {Distribute L1}
	 end
      end
   end
in
   {Distribute {Record.toListInd V}}
end

     
fun {MagicSquare N}
   NN  = N*N
   L1N = {List.number 1 N 1}  % [1 2 3 ... N]
in
   proc {$ Square}
      fun {Field I J}
	 Square.((I-1)*N + J)
      end
      proc {Assert F}
         %% {F 1} + {F 2} + ... + {F N} =: Sum
	 {GFD.sum {Map L1N F} '=:' Sum}
      end
      Sum = {GFD.decl}
   in
      {GFD.tuple square NN 1#NN Square}
      {GFD.distinctP post(Square)}
      %% Diagonals
      {Assert fun {$ I} {Field I I} end}
      {Assert fun {$ I} {Field I N+1-I} end}
      %% Columns
      {For 1 N 1
       proc {$ I} {Assert fun {$ J} {Field I J} end} end}
      %% Rows
      {For 1 N 1
       proc {$ J} {Assert fun {$ I} {Field I J} end} end}
      %% Eliminate symmetries
      /* {Field 1 1} <: {Field N N}
      {Field N 1} <: {Field 1 N}
      {Field 1 1} <: {Field N 1} */
      %% Redundant: sum of all fields = (number rows) * Sum
      %%%%%%%NN*(NN+1) div 2 =: N*Sum
      {GFD.sumC [N] [Sum] '=:' NN*(NN+1) div 2}
      %%
      %{GFD.distribute split Square}
      {NaiveDistribute Square}
   end
end


{Show {SearchOne {MagicSquare 5}}}
%{Show {Search.one.depth {MagicSquare 6} 1 _}}

