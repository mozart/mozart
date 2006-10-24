%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
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

local

   class Toplevel
      from Tk.toplevel
      prop final

      feat
	 manager
	 scale
      
      meth init(Manager Menu Canvas Status ?PackMe)
	 TitleName = {Dictionary.get Manager.options.visual title}
	 Tk.toplevel,tkInit(title:              TitleName
			    'class':            'OzTools'
			    delete:
			       proc {$}
				  {self.manager.status kill}
				  {self.manager close}
			       end
			    withdraw:           true
			    highlightthickness: 0)

	 ScrX  = {New Tk.scrollbar tkInit(parent: self
					  relief: sunken
					  width:  ScrollerWidth
					  orient: horizontal)}
	 ScrY  = {New Tk.scrollbar tkInit(parent: self
					  relief: sunken
					  width:  ScrollerWidth)}
	 Scale = {New Tk.scale tkInit(parent:     self
				      'from':     MinScale
				      to:         MaxScale
				      showvalue:  false
				      width:      ScrollerWidth
				      resolution: 0.001 / FloatScaleBase
				      action:     Manager # scale
				      args:       [float])}
      in
	 Canvas = {New ScrollCanvas init(self Manager)}
	 {Scale tk(set DefScale)}
	 {Scale tkBind(event:'<3>' action: Manager # scaleToFit)}
	 {Tk.addYScrollbar Canvas ScrY}
	 {Tk.addXScrollbar Canvas ScrX}

	 self.manager = Manager
	 self.scale   = Scale

	 proc {PackMe}
	    {Tk.batch [focus(self)
		       wm(iconname self TitleName)
		       wm(minsize  self MinSizeX MinSizeY)
		       grid(Menu   row:0 column:0 columnspan:3 sticky:we)
		       grid(Canvas row:1 column:0              sticky:nswe)
		       grid(ScrY   row:1 column:1              sticky:ns)
		       grid(Scale  row:1 column:2 rowspan:2    sticky:ns)
		       grid(ScrX   row:2 column:0              sticky:we)
		       grid(Status row:3 column:0 columnspan:3 sticky:we)
		       grid(columnconfigure self 0 weight:1)
		       grid(rowconfigure    self 1 weight:1)
		       wm(deiconify self)]}
	 end
      end

   end

   class ScrollCanvas
      from Tk.canvas
      prop final

      attr
	 scale:   DefScale
         %% These values are scaled
	 left:    0
	 right:   0
	 bottom:  0
         %% These values are unscaled
	 width:   1.0
	 height:  1.0
         %% This is the number of the next item to be created
	 item:    0
   
      feat
	 manager
	 actions
	 numbers
	 cursor
	 connection
	 balloon

      meth init(Toplevel Manager)
	 Tk.canvas,tkInit(parent:             Toplevel
			  relief:             sunken
			  width:              StartSizeX
			  height:             StartSizeY
			  background:         BackColor
			  highlightthickness: 0)
	 Tk.canvas,tkBind(event:  '<Configure>'
			  action: self # Resized
			  args:   [float(h) float(w)]
			  append: true)
	 ActionTag = {New Tk.canvasTag tkInit(parent:self)}
	 FloatXY   = [float(x) float(y)]
	 Balloon   = {NewBalloonPort}
      in
	 self.manager    = Manager
	 self.balloon    = Balloon
	 %% Get some tags
	 self.cursor     = {New Tk.canvasTag tkInit(parent:self)}
	 self.connection = {New Tk.canvasTag tkInit(parent:self)}
	 self.numbers    = {New Tk.canvasTag tkInit(parent:self)}
	 self.actions    = ActionTag
	 {ActionTag tkBind(event:  '<1>'
			   args:   FloatXY
			   action: Manager # setByXY)}
	 {ActionTag tkBind(event:  '<Double-1>'
			   args:   FloatXY
			   action: Manager # selInfo)}
	 {ActionTag tkBind(event:  '<2>'
			   args:   FloatXY
			   action: Manager # nodesByXY(hide))}
	 {ActionTag tkBind(event:  '<Double-2>'
			   args:   FloatXY
			   action: Manager # nodesByXY(hideFailed))}
	 {ActionTag tkBind(event:  '<3>'
			   args:   FloatXY
			   action: Manager # doByXY(step))}
	 {ActionTag tkBind(event:  '<Double-3>'
			   args:   FloatXY
			   action: Manager # doByXY(next))}
	 {ActionTag tkBind(event:  '<Enter>'
			   args:   FloatXY
			   action: Balloon # popup(Manager))}
	 {ActionTag tkBind(event:  '<Leave>'
			   action: Balloon # close)}
      end

      meth tkCreate(Tickle)
	 item <- @item + 1
	 Tk.canvas,tk(cr Tickle)
      end
      
      meth tkGet(Tickle ?Id)
	 Id = @item + 1
	 item <- Id
	 Tk.canvas,tk(cr Tickle)
      end
      
      meth clear
	 left   <- 0
	 right  <- 0
	 bottom <- 0
	 Tk.canvas,tk(de all)
	 ScrollCanvas,AdjustRegion
      end

      meth scrollTo(X Y)
	 Scale  = @scale
	 Left   = Scale * {IntToFloat @left}
	 Right  = Scale * {IntToFloat @right}
	 Bottom = Scale * {IntToFloat @bottom}
      in 
	 Tk.canvas,tk(xview moveto
		      ({IntToFloat X}*Scale - Left - @width/2.0)/ (Right-Left))
	 Tk.canvas,tk(yview moveto
		      ({IntToFloat Y}*Scale - @height/2.0)/Bottom)
      end

      meth bounding(NewLeft NewRight NewBottom)
	 left   <- NewLeft
	 right  <- NewRight
	 bottom <- NewBottom
	 ScrollCanvas,AdjustRegion
      end
   
      meth AdjustRegion
	 {Port.send self.balloon close}
	 Scale  = @scale
	 Left   = Scale * {IntToFloat @left}
	 Right  = Scale * {IntToFloat @right}
	 Bottom = Scale * {IntToFloat @bottom}
	 Width  = Right - Left
	 Delta  = (Width - @width) / 2.0
	 ReqLeft # ReqRight = if Delta<0.0 then
				 (Left+Delta) # (Right-Delta)
			      else
				 Left#Right
			      end
	 ReqBottom = if Bottom>@height then Bottom
		     else @height
		     end
      in
	 Tk.canvas,tk(conf scrollregion:q(ReqLeft 0 ReqRight ReqBottom))
      end
      
      meth scale(Scale)
	 ScaleBy = Scale / @scale
      in
	 scale <- Scale
	 ScrollCanvas,AdjustRegion
	 Tk.canvas,tk(scale all 0 0 ScaleBy ScaleBy)
      end

      meth scaleToFit($)
	 Left   = @left
	 Right  = @right
	 Bottom = @bottom
      in
	 if Bottom==0 orelse Left==Right then
	    DefScale
	 else
	    Factor   = {Min @width * FloatScaleBase /
			{IntToFloat (Right - Left)}
			@height * FloatScaleBase /
			{IntToFloat Bottom}} / FloatScaleBase
	    NewScale = if Factor<MinScale then MinScale
		       elseif Factor>MaxScale then MaxScale
		       else Factor
		       end
	 in
	    {self.manager.toplevel.scale tk(set NewScale)}
	    NewScale
	 end
      end
      
      meth Resized(H W)
	 height <- H
	 width  <- W
	 ScrollCanvas,AdjustRegion
      end

      meth postscript(colormode:C rotate:R file:F height:H width:W)
	 Scale  = @scale
	 Height = {IntToFloat @bottom} * Scale
	 Width  = {IntToFloat (@right - @left)} * Scale
      in
	 {Wait Tk.canvas,tkReturn(postscript
				  if H/Height > W/Width then
				     o(pagewidth: W#c)
				  else o(pageheight: H#c)
				  end
				  file:       F
				  colormode:  C
				  rotate:     R
				  height:     Height
				  width:      Width
				  x:          {IntToFloat @left} * Scale
				  y:          0
				  $)}
      end
      
   end


   fun {PickFont Fs Scale}
      case Fs of nil then false
      [] F|Fr then
	 if F.scale=<Scale then F.name else {PickFont Fr Scale} end
      end
   end

   proc {MakeDirty Ns}
      case Ns of nil then skip
      [] N|Nr then
	 case N.mom of false then skip elseof Mom then {Mom dirtyUp} end
	 {MakeDirty Nr}
      end
   end
      
in

   class ToplevelManager
      feat
	 toplevel
	 canvas
      
      attr
	 CurNumber:     1
	 curFont:       {PickFont NumberFonts DefScale}
	 NumberNodes:   nil
	 curNode:       false
	 cmpNode:       false
	 scale:         DefScale

      meth init(?PackMe)
	 self.toplevel  = {New Toplevel init(self
					     self.menu
					     self.canvas
					     self.status ?PackMe)}
      end

      meth clear
	 {self.canvas tk(delete all)}
	 curNode       <- false
	 cmpNode       <- false
	 {self.canvas.numbers tk(delete)}
	 {ForAll @NumberNodes proc {$ N} {N clearNumber} end}
	 CurNumber   <- 1
	 NumberNodes <- nil
      end
      
      meth configurePointer(Status)
	 {Wait {self.canvas
		tkReturn(conf cursor:case Status
				     of drawing   then pencil
				     [] searching then watch
				     [] idle      then top_left_arrow
				     end $)}}
      end

      meth scale(Scale)
	 lock
	    Font    = {PickFont NumberFonts Scale}
	    Canvas  = self.canvas
	    Numbers = Canvas.numbers
	 in
	    scale <- Scale
	    {Canvas scale(Scale)}
	    case @curFont of !Font then skip elseof CF then
	       if @NumberNodes\=nil then
		  if Font==false then {Numbers tk(delete)}
		  elseif CF==false then
		     {ForAll @NumberNodes
		      proc {$ Node}
			 {Node redrawNumber(Scale Font)}
		      end}
		  else {Numbers tk(itemconf font:Font)}
		  end
	       end
	       curFont <- Font
	    end
	 end
      end

      meth scaleToFit
	 lock
	    case {self.canvas scaleToFit($)} of false then skip
	    elseof NewScale then
	       ToplevelManager,scale(NewScale)
	    end
	 end
      end

      meth findByXY(ScaledX ScaledY $)
	 Canvas  = self.canvas
	 CanvasX = {Canvas tkReturnFloat(canvasx(ScaledX) $)}
	 CanvasY = {Canvas tkReturnFloat(canvasy(ScaledY) $)}
	 Scale   = @scale
	 FindX   = {FloatToInt CanvasX / Scale - MaxExtent}
	 Depth   = {FloatToInt
		    {Float.round (CanvasY / Scale - HalfVerSpaceF)
		     / VerSpaceF}}
      in
	 {@root findByX(Depth RootX FindX $)}
      end
      
      meth setCursor(CurNode IsVisible)
	 X Y
	 Scale      = @scale
	 Canvas     = self.canvas
	 Cursor     = Canvas.cursor
	 Connection = Canvas.connection
      in
	 {Cursor tk(delete)}
	 {Connection tk(delete)}
	 {CurNode getCenter(?X ?Y)}
	 {Canvas tkCreate(o({Shapes.(if
					CurNode.kind==choose andthen
					{CurNode isHidden($)} then hidden
				     else CurNode.kind end)
			     {IntToFloat (X + ShadeWidth)} * Scale
			     {IntToFloat (Y + ShadeWidth)} * Scale
			     Scale}
			    fi:CursorColor
			    outline:''
			    ta:Cursor))}
	 {Cursor tk(lower)}
	 if CurNode==@curNode orelse IsVisible then skip else
	    {Canvas scrollTo(X Y)}
	 end
	 curNode <- CurNode
	 case @cmpNode of false then
	    {Connection tk(delete)}
	 elseof CmpNode then
	    if CmpNode\=CurNode then
	       CmpX CmpY
	    in
	       {CmpNode getCenter(?CmpX ?CmpY)}
	       {Canvas
		tkCreate(li(Scale*{IntToFloat X} Scale*{IntToFloat Y}
			    Scale*{IntToFloat CmpX} Scale*{IntToFloat CmpY}
			    arrow: last
			    fi: CursorColor
			    wi: LinkWidth
			    ta: Connection))}
	       {Connection tk('raise')}
	    end
	 end
      end

      meth hideCursor
	 Canvas = self.canvas
      in
	 {Canvas.cursor tk(del)}
	 {Canvas.connection tk(del)}
      end

      meth getNumber(Node ?N)
	 NewNumber = @CurNumber
      in
	 {Node getNumber(@scale @curFont NewNumber ?N)}
	 if NewNumber==N then
	    CurNumber   <- NewNumber + 1
	    NumberNodes <- Node|@NumberNodes
	 end
      end
      
      meth refreshNumbers
	 Numbers = self.canvas.numbers
      in
	 {Numbers tk(itemconfigure fill:LineColor)}
	 {Numbers tk('raise')}
      end

      meth hideNumbers
	 if @curFont\=false andthen @NumberNodes\=nil then
	    {self.canvas.numbers tk(delete)}
	 end
      end

      meth unhideNumbers
	 Font    = @curFont
	 Scale   = @scale
	 Numbers = @NumberNodes
      in
	 if Font\=false andthen Numbers\=nil then
	    {ForAll @NumberNodes
	     proc {$ Node}
		{Node redrawNumber(Scale Font)}
	     end}
	 end
      end
      
      meth makeDirty(Ns)
	 {MakeDirty Ns}
      end
      
      meth close
	 {self.toplevel tkClose}
      end
      
   end
   
end


