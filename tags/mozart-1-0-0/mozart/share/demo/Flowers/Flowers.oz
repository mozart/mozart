%%%
%%% Authors:
%%%   Tobias Mueller (tmueller@ps.uni-sb.de)
%%%
%%% Copyright:
%%%   Tobias Mueller, 1998
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
   Tk
   Application
   
define

   Width = 800.0 Height = 700.0
   
   FlowerA = flower(xPos     : 200.0
		    yPos     : 40.0
		    angle    : 1.570796327
		    start    : x
		    grammar  : grammar(x:[f [l x] f [r x] l x] f:[f f])
		    delta    : 0.383972435
		    stepWidth: 4.0
		    recDepth : 6)
   
   FlowerB = flower(xPos     : 520.0
		    yPos     : 115.0
		    angle    : 1.570796327
		    start    : f
		    grammar  : grammar(f:[f f r [r f l f l f] l [l f r f r f]])
		    delta    : 0.383972435
		    stepWidth: 10.0
		    recDepth : 4)

   proc {TkFlowers}
      W = {New Tk.toplevel tkInit(title: 'Flowers'
				  delete: proc{$}
					     {W tkClose}
					     {Application.exit 0}
					  end)}
      
      F = {New Tk.frame tkInit(parent: W)}
      
      B1 = {New Tk.button tkInit(parent:F action:DrawFlower#FlowerA
				 text:'Start Flower A')}
      B2 = {New Tk.button tkInit(parent:F action:DrawFlower#FlowerB
				 text:'Start Flower B')}
      B3 = {New Tk.button tkInit(parent:F action: proc {$}
						     {DrawFlower FlowerA}
						     {DrawFlower FlowerB}
						  end
				 text:'Start Both')}
      B4 = {New Tk.button tkInit(parent:F action: View#tk(delete all)
				 text:'Clear')}
      View = {New Tk.canvas tkInit(parent:W width:Width height:Height)}
      
      {Tk.batch [wm(minsize W 10 10)
		 pack(B1 B2 B3 B4 side:left)
		 pack(F View)]}
      
      proc {DrawFlower flower(xPos:XPos yPos:YPos angle:Angle
			      start:Start grammar:Grammar delta:Delta
			      stepWidth:StepWidth recDepth: RecDepth)}
	 
	 fun {DrawStep State}
	    state(Xi Yi Z) = !State
	    Xo = Xi + StepWidth * {Cos Z}
	    Yo = Yi + StepWidth * {Sin Z}
	 in
	    {View tk(crea line Xi Height-Yi Xo Height-Yo)}
	    state(Xo Yo Z)
	 end
	 
	 fun {Draw N State ComList}
	    state(X Y Z) = !State
	 in
	    if N > 0 then
	       case ComList of H|T then 
		  case H
		  of l then {Draw N state(X Y Z+Delta) T}
		  [] r then {Draw N state(X Y Z-Delta) T}
		  [] _|_ then 
		     thread {Draw N state(X Y Z) H _} end
		     {Draw N state(X Y Z) T}
		  else {Draw N {Draw N-1 state(X Y Z) Grammar.H} T} end
	       else state(X Y Z) end
	    else {DrawStep state(X Y Z)}
	    end
	 end
      in
	 thread
	    {Draw RecDepth state(XPos YPos Angle) Grammar.Start _}
	 end
      end % proc DrawFlower
   in
      skip
   end % proc TkFlowers
   
   {TkFlowers}
      
end
