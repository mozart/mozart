%  Programming Systems Lab, DFKI Saarbruecken,
%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5315
%  Author: Joerg Wuertz
%  Email: wuertz@dfki.uni-sb.de
%  Last modified: $Date$ by $Author$
%  Version: $Revision$

% Square tiling problem:
% A list of squares of given sizes must fit exactly into a fixed square.

declare Square ShowTiles in

local
   Off=1

   Color = case Tk.isColor then red else black end
   
   Problems = problems(r(s: [18 15 14 10 9 8 7 4 1] sx:32 sy:33)
		       r(s: [3 2 2 1 1 1] sx:5 sy:4)
		       r(s: [50 42 37 35 33 29 27 25 24 19
			     18 17 16 15 11 9 8 7 6 4 2] sx:112 sy:112)
		       r(s: [9 8 8 7 5 4 4 4 4 4 3 3 3 2 2 1 1] sx:20 sy:20)
		       r(s: [6 4 4 4 2 2 2 2] sx:10 sy:10)
		      )

   proc {StateConstraints Choice ?Squares ?SX ?SY}
      SX = Problems.Choice.sx
      SY = Problems.Choice.sy
      Squares = {Map Problems.Choice.s fun{$ S}
					  square(x: {FD.int 0#SX-S}  
						 y: {FD.int 0#SY-S}
						 size: S)
				       end}
   end

   proc {Capacity Squares SX SY Axis}
      {Loop.for 0 SX-1 1 proc{$ Coord}
			    {Sum Coord Squares Axis SY}
			 end}
   end

   proc {Sum Coord Squares Axis SY}
      % the sum of all the heights of rectangles over this position must be SY
      case Squares
      of S|Sr
      then
	 Left = Coord-S.size+1
	 B = case Left < 0 then S.Axis :: 0#Coord
	     else S.Axis :: Left#Coord
	     end
      in
	 SY = {FD.decl}
	 SY =: B*S.size + {Sum Coord Sr Axis}
      [] nil then SY=0
      end
   end

   proc {NoOverlap Squares}
      % No rectangles must overlap
      {ForAllTail Squares proc{$ S1|Sr}
			     {ForAll Sr proc{$ S2}
					   (S1.x+S1.size =<: S2.x) +
					   (S1.x >=: S2.x+S2.size) +
					   (S1.y+S1.size =<: S2.y) +
					   (S1.y >=: S2.y+S2.size) >=: 1
					end}
			  end}
   end


   proc {Enumerate Ls}
      choice
	 case Ls of nil then skip
	 [] L|Lr
	 then Minimum = {FoldL Lr fun{$ I X}
				     M ={FD.reflect.min X} in
				     {Min I M}
				  end {FD.reflect.min L}} 
	 in
	    {SelectSq Ls Minimum nil}
	 end
      end
   end
   proc {SelectSq Ls Minimum Tried}
      case Ls of nil then fail
      [] L|Lr then
	 dis L=Minimum
	 then {Enumerate {Append {Reverse Tried} Lr}}
	 [] L>:Minimum
	 then {SelectSq Lr Minimum L|Tried}
	 end
      end
   end

   fun {Order X Y}
      X.size > Y.size
   end

in
   proc {Square P Squares}
      SX SY Sorted 
   in
      % SX and SY are global sizes
      % The Coordinates give the statring point of the rectangles
      {StateConstraints P Squares SX SY}
      {NoOverlap Squares}
      {Capacity Squares SX SY x}
      {Capacity Squares SY SX y}
      Sorted = {Sort Squares Order}
      {Enumerate {Map Sorted fun{$ S} S.x end}}
      {Enumerate {Map Sorted fun{$ S} S.y end}}
   end

   proc {ShowTiles SX SY Squares Zoom}
      W = {New Tk.toplevel tkInit}
      {Tk.send wm(resizable W 0 0)}
      {Tk.send wm(title(W "Tiling Problems"))}
      Canvas = {New Tk.canvas tkInit(parent:W
				     width:SX*Zoom height:SY*Zoom)}
   in
      {Tk.send pack(Canvas)}
      {ForAll Squares proc{$ S}
			 {Canvas tk(crea(rectangle S.x*Zoom+Off 
					 S.y*Zoom+Off 
					 (S.x+S.size)*Zoom-Off 
					 (S.y+S.size)*Zoom-Off
					 o(fill:Color)))}
		      end}
   end


end


/*
declare  S = {SearchOne
	      proc{$ Squares} 
		 {Square 2 Squares} 
	      end}

declare Ss=S.1 in {ShowTiles 5 4 Ss 40}

declare S = {SearchOne
	     proc{$ Squares} 
		{Square 1 Squares} 
	     end}
declare Ss=S.1 in {ShowTiles 32 33 Ss 10}


declare S = {SearchOne 
	     proc{$ Squares} 
		{Square 3 Squares} 
	     end}
declare Ss=S.1 in {ShowTiles 112 112 Ss 3}

declare S = {SearchOne
	     proc{$ Squares} 
		{Square 4 Squares} 
	     end}
declare Ss=S.1 in {ShowTiles 20 20 Ss 10}

declare S = {SearchOne
	     proc{$ Squares} 
		{Square 5 Squares} 
	     end}
declare Ss=S.1 in {ShowTiles 10 10 Ss 10}

*/
