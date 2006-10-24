%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998, 1999
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
   FD Tk Explorer

export
   Add Delete
   
prepare

   %% Parameters:
   Dist     =  8
   Size     = 70 
   HalfSize = 35 
   WinTitle = 'Knights Board'

   local
      W  = Size
      DM = HalfSize - Dist
      DP = HalfSize + Dist
   in
      fun {GetCoords X1 Y1 X2 Y2}
	 if X1<X2 then
	    if Y1<Y2 then o(X1*W+DP Y1*W+DP X2*W+DM Y2*W+DM)
	    else o(X1*W+DP Y1*W+DM X2*W+DM Y2*W+DP)
	    end
	 else
	    if Y1<Y2 then  o(X1*W+DM Y1*W+DP X2*W+DP Y2*W+DM)
	    else o(X1*W+DM Y1*W+DM X2*W+DP Y2*W+DP)
	    end
	 end	    
      end
   end

define

   Font = {New Tk.font tkInit(family:helvetica weight:bold size:24)}
   
   class KnightsBoard from Tk.canvas
      meth drawJump(N X Y)
	 KnightsBoard,tk(create text X*Size+HalfSize Y*Size+HalfSize
			 fill:black text:N font:Font)
      end
      meth drawSquare(X Y)
	 KnightsBoard,tk(create rectangle
			 X*Size      Y*Size
			 X*Size+Size Y*Size+Size
			 fill:lightgray width:0 outline:gray)
      end
      meth drawSmallArrow(X1 Y1 X2 Y2)
	 KnightsBoard,tk(create line {GetCoords X1 Y1 X2 Y2}
			 fill:dimgray arrow:last width:1)
      end
      meth drawBigArrow(X1 Y1 X2 Y2)
	 KnightsBoard,tk(create line {GetCoords X1 Y1 X2 Y2}
			 fill:red arrow:last width:3)
      end
   end
   
   fun {DrawBoard Node Root}
      Jump   = Root.1
      Succ   = Root.2
      N      = {FloatToInt {Sqrt {IntToFloat {Width Jump}}}}

      fun {FieldToX I}
	 (I-1) mod N
      end
      fun {FieldToY I}
	 (I-1) div N
      end
   
      Window = {New Tk.toplevel tkInit(title:WinTitle#' '#Node)}
      
      Frame  = {New Tk.frame  tkInit(parent:Window relief:ridge)}
      Board  = {New KnightsBoard tkInit(parent:Window relief:sunken
					width:N*Size height:N*Size
					bg:white  bd:2
					highlightthickness:0)}
      Button = {New Tk.button tkInit(parent: Frame
				     text:   'Dismiss'
				     action: Window#tkClose)}
   in   
      {Tk.batch [wm(iconname Window WinTitle#' '#Node)
		 wm(resizable Window 0 0)
		 pack(Button padx:2 side:left)
		 pack(Board Frame padx:4 pady:4)]}

      {For 1 N*N 1
       proc {$ I}
	  X={FieldToX I} Y={FieldToY I}
       in
	  if {IsEven X}=={IsOdd Y} then
	     {Board drawSquare(X Y)}
	  end
       end}

      {Record.forAllInd Succ
       proc {$ I S}
	  if {FD.reflect.size S}>1 then
	     {ForAll {FD.reflect.domList S}
	      proc {$ D}
		 if D\=0 then
		    {Board drawSmallArrow({FieldToX I} {FieldToY I}
					  {FieldToX D} {FieldToY D})}
		 end
	      end}
	  elseif S\=0 then
	     {Board drawBigArrow({FieldToX I} {FieldToY I}
				 {FieldToX S} {FieldToY S})}
	  end
       end} 

      {Record.forAllInd Jump
       proc {$ I J}
	  if {FD.reflect.size J}==1 then
	     {Board drawJump(J {FieldToX I} {FieldToY I})}
	  end
       end}

      Window # tkClose
   end

   proc {Add}
      {Explorer.object add(information separator)}
      {Explorer.object add(information DrawBoard label:'Draw Knights Board')}
   end

   proc {Delete}
      {Explorer.object delete(information DrawBoard)}
   end

end

