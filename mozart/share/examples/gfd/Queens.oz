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

local
   
   % proc {MakeDistrTuple V ?T}
%       T = {VectorToTuple V}
%       if {Record.all T GFD.is} then skip else
% 	 {Exception.raiseError
% 	  kernel(type MakeDistrTuple [V T] 'vector(fd)' 1
% 		 'Distribution vector must contain finite domains.')}
%       end
%    end
	 
   %% Optimized and generic
   SelVal = map(min:      GFD.reflect.min
		max:      GFD.reflect.max
	       )
   
   %% Generic only
   GenSelVar = map(naive:   fun {$ _ _}
			       false
			    end
		   size:    fun {$ X Y}
			       {GFD.reflect.size X}<{GFD.reflect.size Y}
			    end
		   width:   fun {$ X Y}
			       {GFD.reflect.width X}<{GFD.reflect.width Y}
			    end
		   min:     fun {$ X Y}
			       {GFD.reflect.min X}<{GFD.reflect.min Y}
			    end
		   max:     fun {$ X Y}
			       {GFD.reflect.max X}>{GFD.reflect.max Y}
			    end)
	    
   GenSelFil = map(undet:  fun {$ X}
			      {GFD.reflect.size X} > 1
			   end)
   
   GenSelSel = map(id:     fun {$ X}
			      X
			   end)
   
   fun {MapSelect Map AOP}
      if {IsAtom AOP} then Map.AOP else AOP end
   end
	    
   fun {PreProcessSpec Spec}
      FullSpec = {Adjoin
		  generic(order:     size
			  filter:    undet
			  select:    id
			  value:     min
			 )
		  case Spec
		  of naive then generic(order:naive)
		  [] ff    then generic
		  [] split then generic(value:splitMin)
		  else Spec
		  end}
      IsOpt =    case FullSpec
		 of generic(select:id filter:undet procedure:noProc
			    order:OrdSpec value:ValSpec) then
		    {IsAtom OrdSpec} andthen {IsAtom ValSpec}
		 else false
		 end
   in
      if IsOpt then
	 opt(order: FullSpec.order
	     value: FullSpec.value)
      else
	 gen(order:     {MapSelect GenSelVar FullSpec.order}
	     value:     {MapSelect SelVal FullSpec.value}
	     select:    {MapSelect GenSelSel FullSpec.select}
	     filter:    {MapSelect GenSelFil FullSpec.filter})
      end
   end
   
in
	 
   proc {GFDDistribute RawSpec Xs}
      case {PreProcessSpec RawSpec}
      of gen(value:     SelVal
	     order:     Order
	     select:    Select
	     filter:    Fil) then

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
		  LSel = {List.foldL LFil fun {$ I#X Acc} if {Order X Acc} then Acc else I#X end end LFil.1}
		  I#X = LSel
		  M={SelVal X}
	       in
		  {Space.branch [I#M I#compl(M)]}
		  {Distribute LFil}
	       end
	    end
	 end
      in
	 {Distribute {Record.toListInd V}}
      end
   end
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
      
      {GFDDistribute naive Root}
   end
end

{Show {SearchAll {Queens 15}}}
