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
   FD Explorer Tk

export
   Add Delete

define

   
   %% Parameters
   URL           = 'http://www.mozart-oz.org/home/doc/demo/applets/images/animated-queens/' 
   WinTitle      = 'N-Queens'
   ExplorerTitle = 'Draw '#WinTitle
   MaxWidth      = 600
   BlackColor    #
   WhiteColor    #
   QueenColor    #
   CrossColor    = if Tk.isColor then
		      gray85#gray95#darkorange1#gray75
		   else
		      black#white#black#black
		   end

   %% Derived Parameters
   WidthByMag = s(micro:2 tiny:5 small:10 middle:25 large:50)

   QueenByMag = {List.toRecord ''
		 {Map [micro tiny small middle large]
		  fun {$ S}
		     S#{New Tk.image
			tkInit(type:bitmap foreground:QueenColor
			       url: URL # S# '-queen.xbm')}
		  end}}
		  
   CrossByMag = c(micro:  unit
		  tiny:   unit
		  small:  unit
		  middle: {New Tk.image
			   tkInit(type:bitmap foreground:CrossColor 
				  url: URL # 'middle-cross.xbm')}
		  large:  {New Tk.image
			   tkInit(type:bitmap foreground:CrossColor
				  url: URL # 'large-cross.xbm')})

   fun {Comp I W Js}
      case Js of nil then {List.number I W 1}
      [] J|Jr then
	 if J==I then {Comp I+1 W Jr} else I|{Comp I+1 W Js} end
      end
   end
   
   class QueensBoard
      from Tk.canvas

      meth init(parent:P vector:V)
	 S = {Width V}
	 M = if     S*WidthByMag.large =<MaxWidth then large
	     elseif S*WidthByMag.middle=<MaxWidth then middle
	     elseif S*WidthByMag.small =<MaxWidth then small
	     elseif S*WidthByMag.tiny  =<MaxWidth then tiny
	     else micro
	     end
	 W = WidthByMag.M
	 C = CrossByMag.M
	 Q = QueenByMag.M
      in
	 Tk.canvas, tkInit(parent:P width:S*W height:S*W bg:WhiteColor)
	 %% Draw chess board squares
	 {For 0 S-1 2
	  proc {$ I}
	     {For 1 S-1 2
	      proc {$ J}
		 {self tk(create rectangle I*W J*W (I+1)*W (J+1)*W
			  fill:BlackColor outline:'')}
	      end}
	  end}
	 {For 1 S-1 2
	  proc {$ I}
	     {For 0 S-1 2
	      proc {$ J}
		 {self tk(create rectangle I*W J*W (I+1)*W (J+1)*W
			  fill:BlackColor outline:'')}
	      end}
	  end}
	 %% Draw queens
	 {For 1 S 1
	  proc {$ X} Y=V.X in
	     if {IsDet Y} then {self draw(X Y W Q)} end
	  end}
	 %% Draw crosses
	 if C\=unit then
	    {For 1 S 1
	     proc {$ X} Y=V.X in
		if {IsFree Y} then skip else
		   {ForAll {Comp 1 W if {IsDet Y} then [Y]
				     else {FD.reflect.domList Y}
				     end}
		    proc {$ Y}
		       {self draw(X Y W C)}
		    end}
		end
	     end}	    
	 end
      end

      meth draw(X Y W I)
	 Tk.canvas,tk(create image (X-1)*W (Y-1)*W image:I anchor:nw)
      end

   end

   
   fun {Display N S}
      V=if {IsList S} then {List.toTuple '#' S}
	else S
	end
      W={New Tk.toplevel tkInit(title:WinTitle#' ('#N#')')}
      B={New QueensBoard init(parent:W vector:V)}
   in
      {Tk.send pack(B)}
      W#tkClose
   end

   proc {Add}
      {Explorer.object add(information separator)}
      {Explorer.object add(information Display
			   label:ExplorerTitle)}
   end

   proc {Delete}
      {Explorer.object delete(information Display)}
   end

end
