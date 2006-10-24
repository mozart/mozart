%%%
%%% Authors:
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Gert Smolka, 1998
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

declare
Data = [ belgium     # [france netherlands germany luxemburg]
	 germany     # [austria france luxemburg netherlands]
	 switzerland # [italy france germany austria]	 
	 austria     # [italy switzerland germany]
	 france      # [spain luxemburg italy]
	 spain       # [portugal] ]

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
      NbColors  = {FD.decl}
   in
      %% Color: Countries --> 1#NbColors
      {FD.distribute naive [NbColors]}
      {FD.record color Countries 1#NbColors Color}
      {ForAll Data
       proc {$ A#Bs}
	  {ForAll Bs proc {$ B} Color.A \=: Color.B end}
       end}
      {FD.distribute ff Color}
   end
end

{ExploreOne {MapColoring Data}}

