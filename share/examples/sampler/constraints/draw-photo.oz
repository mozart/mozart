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

declare DrawPhoto in

local
   Mid            = 80
   Dist           = 120
   Font           = {New Tk.font tkInit(family:helvetica weight:bold size:24)}
   SmallFont      = {New Tk.font tkInit(family:helvetica weight:bold size:14)}
   FullWidth      = Dist * ({Length Persons} + 1)
   FullHeight     = 280
   HalfHeight     = 140

   fun {ComputeDis N Ful}
      {FoldL {List.zip Prefs Ful fun {$ X Y} X#Y end}
       fun {$ I (A#_)#F}
	  if A==N andthen {FD.reflect.size F}==1 andthen F==0 then I+1
	  else I
	  end
       end 0}
   end

in
   
   fun {DrawPhoto Node Sol}
      NotPos = {Record.filter Sol.pos
		fun {$ P} {FD.reflect.size P}>1 end}

      %% Windowing part
      W = {New Tk.toplevel tkInit(title:'Photo Alignment ('#Node#')')}
      C = {New Tk.canvas   tkInit(parent: W
				  width:  FullWidth
				  height: if {Width NotPos}==0 then
					     HalfHeight
					  else FullHeight
					  end
				  bg:     white)}
      {Tk.send pack(C)}

   in
   
      {Record.forAllInd Sol.pos
       proc {$ N P}
	  if {FD.reflect.size P}==1 then Dis={ComputeDis N Sol.sat} in
	     {C tk(crea text P*Dist Mid o(font:Font text:N))}
	     if Dis>0 then
		{C tk(crea text P*Dist Mid-30
		      font:SmallFont text:~Dis fill:red)}
		{C tk(crea oval P*Dist - 15 Mid-45  P*Dist + 15 Mid-15
		      outline:red width:2)}
	     end
	  end
       end}

      {Record.foldLInd NotPos
       fun {$ N I P}
	  Dis={ComputeDis N Sol.sat}
       in
	  {C tk(crea text I 2*Mid font:Font text:N fill:gray)}
	  if Dis>0 then 
	     {C tk(crea text I*Dist 2*Mid+30
		   font:SmallFont text:~Dis fill:red)}
	     {C tk(crea oval I*Dist - 15 Mid+45  I*Dist + 15 Mid+15
		   outline:red width:1)}
	  end
	  {ForAll {FD.reflect.domList P}
	   proc {$ To}
	      {C tk(crea line I 2*Mid - 20 To*Dist Mid + 20
		    arrow:last width:1 fill:gray)}
	   end}
	  I+Dist
       end
       (FullWidth - {Width NotPos} * Dist + Dist) div 2
       _}

      {ForAll {List.zip Prefs Sol.sat fun {$ X Y} X#Y end}
       proc {$ (NA#NB)#F}
	  if {FD.reflect.size F}>1 then skip
	  elseif F==1 then
	     L = {Min Sol.pos.NA Sol.pos.NB}
	     R = {Max Sol.pos.NA Sol.pos.NB}
	  in
	     {C tk(create line
		   L*Dist + 40           Mid
		   R*Dist - 40           Mid
		   arrow: if {Member NB#NA Prefs} andthen
			     (F2={Nth Sol.sat {List.foldLInd Prefs
					       fun {$ I P A#B}
						  if A#B==NB#NA then I
						  else P
						  end
					       end 0}}
			    in {FD.reflect.size F2}==1 andthen F2==1)
			  then both
			  elseif Sol.pos.NA < Sol.pos.NB then last
			  else first
			  end
		   width: 3
		   fill:  blue)}
	  end
       end}
      
      W # tkClose
   end

end


	  
