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

export
   GetDist
   GetDetourDist
   GetRoute

   Cities
   IsCity
   GetGraph

   width:    GermanyWidth
   height:   GermanyHeight
   GetCoord
   
prepare

   Germany = \insert 'Germany.oz'

   GermanyWidth  = Germany.width
   GermanyHeight = Germany.height
   
   fun {GetDist Src Dst}
      if Src==Dst then 0
      else V=Germany.map.Src.Dst in
	 if {IsInt V} then V else V.1 end
      end
   end

   fun {GetDetourDist Src Via Dst}
      if Via==Dst orelse Via==Src then 0
      elseif Src==Dst then 2*{GetDist Src Via}
      else
	 %% Due to sampling errors in the map data the detour might be
	 %% negative!
	 {Max {GetDist Src Via} + {GetDist Via Dst} - {GetDist Src Dst} 0}
      end
   end

   fun {GetRoute Src Dst}
      if Src==Dst then [Dst#0]
      else SrcDst=Germany.map.Src.Dst in
	 if {IsInt SrcDst} then [Src#SrcDst Dst#0]
	 else Via=SrcDst.2 in Src#Germany.map.Src.Via|{GetRoute Via Dst}
	 end
      end
   end

   Cities = {Arity Germany.map}

   local
      fun {MkSrc Cs F}
	 case Cs of nil then nil
	 [] C|Cr then
	    if {IsInt Germany.map.F.C} then Germany.coord.C|{MkSrc Cr F}
	    else {MkSrc Cr F}
	    end
	 end
      end

      fun {MkGraph Cs}
	 case Cs of nil then nil
	 [] C|Cr then C#Germany.coord.C#{MkSrc Cr C}|{MkGraph Cr}
	 end
      end
   in
      fun {GetGraph}
	 {MkGraph Cities}
      end
   end

   fun {IsCity A}
      {HasFeature Germany.map A}
   end


   proc {GetCoord C ?X ?Y}
      X#Y=Germany.coord.C
   end
   
end

