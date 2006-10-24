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
   
   class WindowClass from BaseObject
      feat width height win view frame button1 button2
      attr color
      meth init(Width Height Title Draw) View in 
	 self.width = {IntToFloat Width}
	 self.height = {IntToFloat Height}
	 color <- black
	 self.win = {New Tk.toplevel tkInit(title: '3D Flowers'
					    delete: proc{$}
						       {self.win tkClose}
						       {Application.exit 0}
						    end)}
	 self.view = {New Tk.canvas tkInit(parent: self.win bg: black 
					   width: Width height: Height)}
	 View = self.view
	 {Tk.send wm(title self.win Title)}
	 {Tk.send wm(minsize self.win 10 10)}

	 self.frame = {New Tk.frame tkInit(parent: self.win
					   bg: {TkColor blue})}
	 self.button1 = {New Tk.button tkInit(parent: self.frame action: Draw
					      bg: green 
					      text:"Start DrawingFlower")}
	 self.button2 = {New Tk.button tkInit(parent: self.frame
					      bg: {TkColor red}
					      action: proc {$} 
							 {View tk(delete(all))}
						      end
					      text:"Clear")}
	 {Tk.send pack(self.button1 self.button2 side:left)}
	 
	 {Tk.send pack(self.frame self.view)}
      end
      meth drawLine(X1 Y1 X2 Y2)
	 {self.view tk(crea line X1 (self.height-Y1)
		       X2 (self.height-Y2) fill: @color)}
      end
      meth ConvertPoints(In $)
	 Height = self.height
      in 
	 b({FoldR In fun{$ X#Y In}X|(Height-Y)|In end nil})
      end
      meth filledPolygon(Points)
	 {self.view tk(crea polygon {self ConvertPoints(Points $)}
		       fill: @color)}
      end
      meth setColor(Color)
	 color <- Color
      end
   end % class WindowClass

   class TurtleObject from BaseObject
      feat sw a win 
      attr x y pl tuX tuY tuZ tlX tlY tlZ thX thY thZ 
      meth init(Win IState)
	 x   <- IState.istate.start.x 
	 y   <- IState.istate.start.y
	 pl  <- nil
	 tuX <- IState.istate.tu.x
	 tuY <- IState.istate.tu.y
	 tuZ <- IState.istate.tu.z
	 tlX <- IState.istate.tl.x
	 tlY <- IState.istate.tl.y
	 tlZ <- IState.istate.tl.z
	 thX <- IState.istate.th.x
	 thY <- IState.istate.th.y
	 thZ <- IState.istate.th.z
	 self.sw  = IState.stepWidth
	 self.a   = IState.arc 
	 self.win = Win 
      end 

      meth init1(X Y PL TuX TuY TuZ TlX TlY TlZ ThX ThY ThZ SW Arc Win)
	 x   <- X 
	 y   <- Y 
	 pl  <- PL
	 tuX <- TuX 
	 tuY <- TuY 
	 tuZ <- TuZ 
	 tlX <- TlX 
	 tlY <- TlY
	 tlZ <- TlZ 
	 thX <- ThX
	 thY <- ThY
	 thZ <- ThZ
	 self.sw  = SW
	 self.a   = Arc 
	 self.win = Win 
      end 

      meth duplicate($)
	 {New TurtleObject init1(@x @y @pl
				 @tuX @tuY @tuZ @tlX @tlY @tlZ 
				 @thX @thY @thZ
				 self.sw self.a  self.win)}
      end

      meth move
	 x <- (@x + self.sw * @thX)
	 y <- (@y - self.sw * @thY) 
	 pl <- (@x#@y)|@pl
      end 

      meth openPoly
	 pl <- (@x#@y)|@pl
      end 
 
      meth closePoly(C)
	 {self.win setColor(C)}
	 {self.win filledPolygon(@pl)}
	 pl <- nil
      end 

      meth draw(C) X = @x Y = @y in
	 x <- @x + self.sw * @thX
	 y <- @y - self.sw * @thY 
	 {self.win setColor(C)}
	 {self.win drawLine(X Y @x @y)}
      end

      meth VM(X1 Y1 Z1 X2 Y2 Z2 A B XR YR ZR)
	 XR <- A * X1 + B * X2
	 YR <- A * Y1 + B * Y2
	 ZR <- A * Z1 + B * Z2
      end
      
      meth turnU
	 TurtleObject,VM(@thX @thY @thZ @tlX @tlY @tlZ ~1.0 0.0 thX thY thZ)
	 TurtleObject,VM(@thX @thY @thZ @tlX @tlY @tlZ 0.0 ~1.0 tlX tlY tlZ)
      end 

      meth rotUpos
	 TurtleObject,VM(@thX @thY @thZ @tlX @tlY @tlZ {Cos self.a} ~{Sin self.a} thX thY thZ)
	 TurtleObject,VM(@thX @thY @thZ @tlX @tlY @tlZ {Sin self.a} {Cos self.a} tlX tlY tlZ)
      end 

      meth rotUneg
	 TurtleObject,VM(@thX @thY @thZ @tlX @tlY @tlZ {Cos self.a} {Sin self.a} thX thY thZ)
	 TurtleObject,VM(@thX @thY @thZ @tlX @tlY @tlZ ~{Sin self.a} {Cos self.a} tlX tlY tlZ)
      end

      meth rotLpos
	 TurtleObject,VM(@thX @thY @thZ @tuX @tuY @tuZ {Cos self.a} {Sin self.a} thX thY thZ)
	 TurtleObject,VM(@thX @thY @thZ @tuX @tuY @tuZ ~{Sin self.a} {Cos self.a} tuX tuY tuZ)
      end

      meth rotLneg
	 TurtleObject,VM(@thX @thY @thZ @tlX @tlY @tlZ {Cos self.a} ~{Sin self.a} thX thY thZ)
	 TurtleObject,VM(@thX @thY @thZ @tlX @tlY @tlZ {Sin self.a} {Cos self.a} tuX tuY tuZ)
      end

      meth rotHpos
	 TurtleObject,VM(@tlX @tlY @tlZ @tuX @tuY @tuZ {Cos self.a} {Sin self.a} tlX tlY tlZ)
	 TurtleObject,VM(@tlX @tlY @tlZ @tuX @tuY @tuZ ~{Sin self.a} {Cos self.a} tuX tuY tuZ)
      end 
      
      meth rotHneg
	 TurtleObject,VM(@tlX @tlY @tlZ @tuX @tuY @tuZ {Cos self.a} ~{Sin self.a} tlX tlY tlZ)
	 TurtleObject,VM(@tlX @tlY @tlZ @tuX @tuY @tuZ {Sin self.a} {Cos self.a} tuX tuY tuZ)
      end
   end % class TurtleObject

   proc {Flower3D Turtle IState}
      Start = IState.start Grammar = IState.grammar
      Colors = IState.colors RecDepth = IState.recDepth
      proc {Interpret Color Turtle N ComList}
	 if N > 0 then
	    case ComList
	    of H|T then
	       case H 
	       of '+' then {Turtle rotUpos}
	       [] '-' then {Turtle rotUneg}
	       [] '|' then {Turtle turnU}  
	       [] '\&' then {Turtle rotLpos}
	       [] '^' then {Turtle rotLneg}
	       [] '\\' then {Turtle rotHpos}
	       [] '/' then {Turtle rotHneg}
	       [] _|_ then NewTurtle = {Turtle duplicate($)} in 
		  thread {Interpret Color NewTurtle N H} end
	       [] 'F' then {Turtle draw(Color)}     
	       [] 'f' then {Turtle move}     
	       [] '{' then {Turtle openPoly} 
	       [] '}' then {Turtle closePoly(Color)}
	       else
		  {Interpret Colors.H Turtle N-1 Grammar.H}
	       end 
	       {Interpret Color Turtle N T}
	    else skip
	    end 
	 else skip
	 end 
      end 
   in	 
      {Interpret Colors.Start Turtle RecDepth Grammar.Start}
   end % proc Flower3D

   fun {TkColor C} if Tk.isColor then C else white end end
   
   Plant = plant(start  : plant
		 grammar: grammar(
			     plant: [internode '+' [plant '+' flower]'-'
				     '-' '/' '/' ['-' '-'leaf] internode
				     ['+' '+'leaf]'-' [plant '-' '-'
						       flower]'+'
				     '+' plant flower]
			     internode: ['F' seg ['/' '/' '&' '&' leaf]
					 ['/'leaf '/' '^' '^'leaf]
					 'F' seg]
			     seg: [seg 'F' seg]
			     leaf: [['{' '+'f f'-'f f f'-'f f'+' '|' 
				     '+'f'-'f f'-'f'}']]
			     flower: [['&' '&' '&'pedicel'/'wedge
				       '/' '/' '/' '/'
				       wedge'/' '/' '/' '/'wedge
				       '/' '/' '/' '/'
				       wedge'/' '/' '/' '/' wedge]]
			     pedicel: ['F' 'F']
			     wedge: [stamen['{' '&' '&' '&' '&' 
					    '-'f f f f'+'f f f'|'
					    '-'f f f'+'f f f f'}']]
			     stamen: [['^' '{' '-' f f f '+'f f '|'
				       '-' f f f '+'f f  '}']]
			     )
		 colors: colors(plant    : {TkColor green}
				internode: {TkColor brown}
				seg      : {TkColor brown}
				leaf     : {TkColor darkgreen}
				flower   : {TkColor brown}
				pedicel  : {TkColor brown}
				wedge    : {TkColor red}
				stamen   : {TkColor yellow})
		 istate:state(start: start(x: 100.0 y: 20.0)
			      tu: tu(x: ~10.0 y: ~2.0 z: 3.0)
			      tl: tl(x: 6.0 y: 2.0 z: ~5.0)
			      th: th(x: 1.0 y: ~10.0 z: 0.0))
		 arc    : 0.31
		 stepWidth: 1.2
		 recDepth : 5
		) % Plant

   proc {TkFlowers3D} Win in
      Win = {New WindowClass init(450 750 'Lindenmayer Flower '
				  proc {$}
				     {Flower3D {New TurtleObject
						init(Win Plant)} Plant}
				  end)}
   end

   {TkFlowers3D}

end 
