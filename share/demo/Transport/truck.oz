%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

class Truck from Tk.canvasTag
   prop final
   feat
      parent load fill driver
   attr
      city: unit
      turn: left
      load: 0
      x:    0.0
      y:    0.0
   
   meth init(parent:P city:C driver:D)
      X # Y = Country.coord.C
   in
      city <- C
      Truck, tkInit(parent:P)
      x <- {IntToFloat X}
      y <- {IntToFloat Y}
      self.parent = P
      self.load   = {New Tk.canvasTag tkInit(parent:P)}
      self.fill   = {GetFillColor}
      self.driver = D
      Truck, draw
   end
   
   meth draw
      P = self.parent
      D = @turn
      X = @x
      Y = @y
   in
      Truck, tk(delete)
      if Tk.isColor then
	 %% Create the truck's window
	 {P tk(create image X Y 
	       image: Images.truck.win.D
	       tags:  self)}
	 {P tk(create image X Y
	       image: Images.truck.fill.(self.fill).D
	       tags:  self)}
      end
      %% Create the frame for truck (better visibility) 
      {P tk(create image X Y 
	    image: Images.truck.frame.D
	    tags:  self)}
      {P tk(crea rectangle 0 0 0 0
	    fill:    GoodColor
	    outline: ''
	    tags:    q(self self.load))}
      Truck, load(@load)
   end
   
   meth load(L)
      W  = {IntToFloat L} / {IntToFloat Capacity} * LoadWidth
      X0 = @x + case @turn
		of left  then LoadLeftX
		[] right then LoadRightX + LoadWidth - W
		end
      X1 = X0 + W
      Y0 = @y + LoadY
      Y1 = Y0 + LoadHeight
   in
      load <- L
      {self.load tk(coords X0 Y0 X1 Y1)}
   end
   
   meth turn(X0 X1)
      NewTurn = if X0<X1 then right else left end
   in
      if @turn\=NewTurn then
	 turn <- NewTurn Truck, draw
      end
   end
   
   meth drive(Dst Load NextLoad)
      X#Y = Country.coord.@city
   in
      Truck,load(Load)
      Truck,Route({Country.getRoute @city Dst}
		  {IntToFloat X} {IntToFloat Y})
      Truck,load(NextLoad)
      Truck,{self.driver getMessage($)}
   end
   
   meth Move(N XS YS)
      if N\=0 then
	 Truck,tk(move XS YS)
	 x <- @x + XS y <- @y + YS
	 {Delay DelayMove}
	 Truck,Move(N-1 XS YS)
      end
   end
   
   meth Route(Rs SrcX SrcY)
      %% Moves the truck according to the route "Rs"
      Src#Dist|Rr = Rs
   in
      case Rr of Dst#_|_ then
	 Steps = Dist div Delta
	 Ratio = {IntToFloat Steps}
	 DX#DY = Country.coord.Dst
	 DstX  = {IntToFloat DX}
	 DstY  = {IntToFloat DY}
      in
	 %% Turn the truck
	 Truck,turn(SrcX DstX)
	 Truck,Move(Steps (DstX - SrcX) / Ratio (DstY - SrcY) / Ratio)
	 %% correct
	 Truck,tk(move DstX - @x DstY - @y)
	 x <- DstX
	 y <- DstY
	 Truck,Route(Rr DstX DstY)
      [] nil then
	 city <- Src
      end
   end

   meth close
      Truck, tkClose
   end
   
end	 
