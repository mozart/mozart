%%%
%%% Authors:
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Gert Smolka, 1998
%%%
%%% Last change:
%%%   $Date: 1999/01/18 21:56:05 $ by $Author: schulte $
%%%   $Revision: 1.2 $
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
	 of lt(M) then V.I <: M
	 [] gt(M) then V.I >: M
	 [] eq(M) then V.I =: M	   
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
	    {Space.branch [I#eq(M) I#lt(M) I#gt(M) ]}
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
      {GFD.distinct Square GFD.cl.val}
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

