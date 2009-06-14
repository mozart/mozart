%%%
%%% Authors:
%%%   Andres Felipe Barco <anfelbar@univalle.edu.co>
%%%
%%% Copyright:
%%%   Andres Felipe Barco, 2008
%%%
%%% Last change:
%%%   $Date: $ by $Author:$
%%%   $Revision:$
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

%%% This example containts a implementation of a distributor. This distributor
%%% works in the old style way, meaning usign the Space.waitStable primitive to
%%% perform distribution. We only provide this like an example of
%%% how can you create your own old style distributor.

declare

%%% Value selection strategie
SelVal = map(min: GFD.reflect.min
	     max: GFD.reflect.max
	    )

%% Generic only
%%% Variable selection strategie
GenSelVar = map(naive:   fun {$ _ _}
			    false
			 end
		size:   fun {$ X Y}
			   {GFD.reflect.size X} < {GFD.reflect.size Y}
			end
	       )

%%% Filter function for variables (not determined one's)
GenSelFil = map(undet:  fun {$ X}
			   {Bool.'not' {IsDet X}} andthen ({GFD.reflect.size X} > 1)			      
			end)

GenSelPro = map(noProc: unit)
   
GenSelSel = map(id:     fun {$ X}
			   X
			end)
   
fun {MapSelect Map AOP}
   if {IsAtom AOP} then Map.AOP else AOP end
end

%%% Process wich strategie combination has been choosen.
fun {PreProcessSpec Spec}
   FullSpec = {Adjoin
	       generic(order:     size
		       filter:    undet
		       select:    id
		       value:     min
		       procedure: noProc)
	       case Spec
	       of naive then generic(order:naive)
	       [] ff    then generic
	       [] splitUpper then generic(value:splitUpper)
	       [] rand then generic(order:rand)
	       [] bool then generic(value:bool)
	       else Spec
	       end}
in
   gen(order:     {MapSelect GenSelVar FullSpec.order}
       value:     {MapSelect SelVal FullSpec.value}
       select:    {MapSelect GenSelSel FullSpec.select}
       filter:    {MapSelect GenSelFil FullSpec.filter}
       procedure: {MapSelect GenSelPro FullSpec.procedure})
end
   

%% Returns the filtered list of vars
%% as well as the chosen variable.
fun {ChooseAndRetFiltVars Vars Order Filter}
   NewVars
   fun {Loop Vars Accu NewTail}
      case Vars of nil then
	 NewTail=nil
	 Accu|NewVars
      [] H|T then
	 if {Filter H} then LL in NewTail=(H|LL)
	    {Loop T
	     if Accu==unit orelse {Order H Accu}
	     then H else Accu end
	     LL}
	 else {Loop T Accu NewTail} end
      end
   end
in
   {Loop Vars unit NewVars}
end

%%% Old Style IntVar Distributor
proc {IntVarDistribute RawSpec Vec}
   case {PreProcessSpec RawSpec}
   of gen(value:     SelVal
	  order:     Order
	  select:    Select
	  filter:    Fil
	  procedure: Proc) then
      if {Width Vec}>0 then
	 
	 proc {Do Xs}
	    {Space.waitStable}
	    E|Fs={ChooseAndRetFiltVars Xs Order Fil}
	 in
	    if E\=unit then
	       V={Select E}
	    in
	       if Proc\=unit then
		  {Proc}
		  {Space.waitStable}
	       end		  
	       choice
		  {GFD.relP post(V GFD.rt.'=:' {SelVal V} cl:GFD.cl.bnd)}
	       []
		  {GFD.relP post(V GFD.rt.'\\=:' {SelVal V} cl:GFD.cl.bnd)}
	       end		  
	       {Do Fs}
	    end
	 end
      in
	 {Do {Record.toList Vec}}
      end
   end
end


%%% Adapted from a finite domain example in Mozart-Oz version 1.3.2 by 
%%% Gert Smolka, 1998.
%%% This is an latin american map.

Data = [ colombia    # [panama ecuador peru venezuela brasil]
	 peru        # [ecuador brasil colombia bolivia chile]
	 argentina   # [chile uruguay paraguay bolivia]	 
	 chile       # [peru bolivia argentina]
	 paraguay    # [bolivia argentina brasil]
	 brasil      # [bolivia venezuela]]

fun {Unique Xs}
   case Xs of X1|X2|Xr then
      if X1==X2 then {Unique X2|Xr} else X1|{Unique X2|Xr} end
   else Xs
   end
end
fun {MapColoring Data}
   Countries = {Unique
		{Sort
		 {FoldR Data fun {$ C#Cs A} {Append Cs C|A} end nil}
		 Value.'<'}}
in
   proc {$ Color}
      NbColors
   in
      %% Color: Countries --> 1#NbColors
      NbColors :: 1#GFD.sup
      {GFD.distribute naive [NbColors]}
      {GFD.record color Countries 1#NbColors Color}
      {ForAll Data
       proc {$ A#Bs}
	  {ForAll Bs proc {$ B} {GFD.relP post(Color.A GFD.rt.'\\=:' Color.B)} end}
       end}
      %{GFD.distribute ff Color}
      {IntVarDistribute ff Color}
   end
end

%{ExploreOne {MapColoring Data}}
{Browse {SearchOne {MapColoring Data}}}

