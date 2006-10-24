%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
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

functor

import
   Search(base)
   FD
   Country(getDist getDetourDist)
   Configure(capacity: Capacity)
   
export
   make: MakePlan

define

   %%
   %% Defaults used
   %%

   
   proc {InsertSrc JA|Js Src Dst Weight What ?C ?NJs}
      case Js of nil then
	 if Src==JA.city then 
	    NJs = [act(city:Src
		       load:Weight 
		       lift:What|JA.lift
		       drop:JA.drop)
		   act(city:Dst
		       load:0     
		       lift:nil
		       drop:[What])]
	    C   = {Country.getDist Src Dst}
	 else
	    NJs = [JA
		   act(city: Src
		       load: Weight
		       lift: [What]
		       drop: nil) 
		   act(city: Dst
		       load: 0
		       lift: nil
		       drop: [What])]
	    C   = ({Country.getDist JA.city Src}
		   + {Country.getDist Src Dst})
	 end
      [] JB|_ then
	 NewLoad = JA.load + Weight
      in
	 if NewLoad>Capacity then
	    %% Job cannot placed here, because load would exceed the
	    %% capacity of the truck
	    NJs=JA|{InsertSrc Js Src Dst Weight What ?C}
	 else
	    dis NewJob in
	       if Src==JA.city then
		  %% Okay, the truck drives to this city anyway, so
		  %% simply add this good
		  NJs = (act(city: Src
			     load: NewLoad
			     lift: What|JA.lift
			     drop: JA.drop) = NewJob)
		      | {InsertDst NewJob Js Dst Weight What ?C}
	       else TmpTour={FD.int 0#FD.sup} in
		  %% Add the city to the towns the truck has to visit
		  NJs = (JA 
			 | (act(city: Src
				load: NewLoad
				lift: [What]
				drop: nil) = NewJob)
			 | {InsertDst NewJob Js Dst Weight What ?TmpTour})
		  C =: TmpTour
		       + {Country.getDetourDist JA.city Src JB.city}
	       end
	    [] % ...or somewhere else
	       NJs=JA|{InsertSrc Js Src Dst Weight What ?C}
	    end
	 end
      end
   end

   proc {InsertDst JA Js Dst Weight What ?C ?NJs}
      %% JA is loaded with Order. Order.to has to be placed 
      %% after JA either between planned cities or on planned
      %% cities. Recursion tries to keep Order transported up to 
      %% cities later in Plan
      case Js of nil then
	 C   = {Country.getDist JA.city Dst}
	 NJs = [act(city: Dst
		    load: 0
		    lift: nil
		    drop: [What])]
      [] JB|Jr then
	 dis
	    if JB.city==Dst then
	       C   = 0
	       NJs = {AdjoinAt JB drop What|JB.drop} | Jr
	    else
	       C   = {Country.getDetourDist JA.city Dst JB.city}
	       NJs = act(city: Dst
			 load: JA.load-Weight
			 lift: nil
			 drop: [What]) | Js
	    end
	 [] NewLoad = JB.load + Weight
	    NewJob  = {AdjoinAt JB load NewLoad}
	 in
	    NewLoad =<: Capacity
	 then
	    NJs = NewJob | {InsertDst NewJob Jr Dst Weight What ?C}
	 end
      end
   end

   proc {MakePlan Tour Task ?Cost ?NewTour}
      Cost # NewTour = case {Search.base.best
			     proc {$ S}
				C # NewTour = S
			     in
				C :: 0#FD.sup
				{InsertSrc Tour Task.src Task.dst Task.weight
				 Task.what ?C ?NewTour}
			     end
			     proc {$ SA SB} SA.1>:SB.1 end}
		       of nil then false#Tour
		       [] [S] then S
		       end
   end

end
