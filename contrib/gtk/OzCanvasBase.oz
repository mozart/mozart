%%%
%%% Author:
%%%   Thorsten Brunklaus <bruni@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2001
%%%
%%% Last Change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local
   AssignArgs = {NewName}
   MakePoints = {NewName}
in
   class OzCanvasBase from GtkLayout
      meth !MakePoints(Points I Ps $)
	 case Ps
	 of (X#Y)|Pr then
	    {GOZCore.pointsPut Points I X _}
	    {GOZCore.pointsPut Points (I + 1) Y _}
	    OzCanvasBase, MakePoints(Points (I + 2) Pr $)
	 [] nil then Points
	 end
      end
      meth !AssignArgs(Item Args)
	 case Args
	 of (Par#RawVal)|Ar then
	    ParVal = case Par
		     of "points" then
			Points = {self pointsNew({Length RawVal} $)}
		     in
			OzCanvasBase, MakePoints({O2P Points} 0 RawVal $)
		     [] _ then RawVal
		     end
	 in
	    {Item set(Par ParVal)}
	    OzCanvasBase, AssignArgs(Item Ar)
	 [] nil then skip
	 end
      end
      meth newItem(Group Type Args $)
	 Item = {New CanvasItem new(Group Type unit {GOZCore.null})}
      in
	 OzCanvasBase, AssignArgs(Item Args)
	 Item
      end
      meth configureItem(Item Args)
	 OzCanvasBase, AssignArgs(Item Args)
      end
      meth closeItem(Item)
	 {Item close}
      end
   end
end
