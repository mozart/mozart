%%%
%%% Authors:
%%%     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
%%%
%%% Copyright:
%%%     Gustavo Gutierrez, 2006
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

   
%% GeIntVar Distribution. var selection and val selection
   IvdVarSel = map( naive:      0
                    size:       1
                    min:        2
                    max:        3
                    nbProp:     4
                    width:      5
                  )

   IvdValSel = map( min:        0
                    mid:        1
                    max:        2
                    splitMin:   3
		    splitMax:   4                                                                                                                               )
	 
	 
%% Optimized and generic
SelVal = map(min:      FdReflect.min
	     max:      FdReflect.max
	     splitMin: FdReflect.med
	    )
   
%% Generic only
GenSelVar = map(naive:   fun {$ _ _}
			    false
			 end
		size:    fun {$ X Y}
			    {FdReflect.size X}<{FdReflect.size Y}
			 end
		width:   fun {$ X Y}
			    {FdReflect.width X}<{FdReflect.width Y}
			 end
		min:     fun {$ X Y}
			    {FdReflect.min X}<{FdReflect.min Y}
			 end
		max:     fun {$ X Y}
			    {FdReflect.max X}>{FdReflect.max Y}
			 end)
	    
GenSelFil = map(undet:  fun {$ X}
			   {FdReflect.size X} > 1
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
	      of generic(select:id filter:undet %procedure:noProc
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
   	 
proc {GFDDistribute RawSpec Xs}
   case {PreProcessSpec RawSpec}
   of opt(value:SelVal order:SelVar) then
      {Wait {GFDP.distribute IvdVarSel.SelVar IvdValSel.SelVal Xs}}
   [] gen(value:     SelVal
	  order:     Order
	  select:    Select
	  filter:    Fil) then
      
      V = if {IsList Xs} then {List.toTuple '#' Xs} else Xs end
      proc {Distribute L}
	 case {Space.getChoice}
	 of I#D then
	    case D
	    of compl(M) then
	       {RelP post(V.I Rt.'\\=:' M cl:Cl.bnd)}
	    [] M then
	       {RelP post(V.I Rt.'=:' M cl:Cl.bnd)}
	    end
	    {Distribute L}
	 [] nil then
	    LFil = {List.filter L fun {$ I#X} {Fil X} end}
	 in
	    if LFil \= nil then
	       LSel = {List.foldL LFil
		       fun {$ I#X O#Acc}
			  if {Order X Acc} then O#Acc
			  else I#X end end LFil.1}
	       
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

